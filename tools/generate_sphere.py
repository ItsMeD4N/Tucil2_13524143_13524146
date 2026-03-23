"""
generate_sphere.py — Generator model bola (sphere) dalam format .obj
Dipakai untuk stress test program Voxelizer.

Cara pakai:
    python tools/generate_sphere.py [output_path] [subdivisions]
    
Contoh:
    python tools/generate_sphere.py test/sphere.obj 32
    
Parameter:
    output_path  : Path file .obj output (default: test/sphere.obj)
    subdivisions : Jumlah pembagian latitude dan longitude (default: 32)
                   Semakin besar = semakin halus dan banyak segitiga.
                   16 = 480 tri, 32 = 1984 tri, 64 = 8064 tri, 128 = 32512 tri
"""

import sys
import math
import os


def generate_sphere(filepath, stacks, slices, radius=1.0):
    """
    Menghasilkan file .obj berisi bola (UV sphere).
    
    Prinsip:
    - Bola dibagi menjadi 'stacks' lapisan horizontal (latitude)
      dan 'slices' potongan vertikal (longitude).
    - Setiap titik dihitung menggunakan koordinat bola:
        x = r * sin(phi) * cos(theta)
        y = r * cos(phi)
        z = r * sin(phi) * sin(theta)
      di mana phi = sudut dari kutub atas (0 sampai pi),
              theta = sudut rotasi horizontal (0 sampai 2*pi).
    - Kutub atas dan bawah menggunakan triangle fan,
      sisanya menggunakan quad yang dipecah jadi 2 segitiga.
    """
    
    # Pastikan folder output ada
    dirpath = os.path.dirname(filepath)
    if dirpath:
        os.makedirs(dirpath, exist_ok=True)
    
    vertices = []
    faces = []
    
    # Generate vertices
    for i in range(stacks + 1):
        phi = math.pi * i / stacks  # 0 (kutub atas) → pi (kutub bawah)
        for j in range(slices + 1):
            theta = 2.0 * math.pi * j / slices  # 0 → 2*pi
            
            x = radius * math.sin(phi) * math.cos(theta)
            y = radius * math.cos(phi)
            z = radius * math.sin(phi) * math.sin(theta)
            
            vertices.append((x, y, z))
    
    # Generate faces (segitiga)
    # Setiap "kotak" di grid latitude-longitude dipecah menjadi 2 segitiga
    for i in range(stacks):
        for j in range(slices):
            # Indeks 4 pojok kotak (1-based untuk .obj)
            #   p1 --- p2
            #    |      |
            #   p3 --- p4
            p1 = i * (slices + 1) + j + 1        # baris atas, kolom kiri
            p2 = i * (slices + 1) + (j + 1) + 1  # baris atas, kolom kanan
            p3 = (i + 1) * (slices + 1) + j + 1  # baris bawah, kolom kiri
            p4 = (i + 1) * (slices + 1) + (j + 1) + 1  # baris bawah, kolom kanan
            
            # Segitiga 1: p1 → p3 → p2 (CCW winding)
            # Segitiga 2: p2 → p3 → p4 (CCW winding)
            # Hindari degenerate triangle di kutub
            if i != 0:
                faces.append((p1, p3, p2))
            if i != stacks - 1:
                faces.append((p2, p3, p4))
    
    # Tulis ke file .obj
    with open(filepath, 'w') as f:
        f.write(f"# UV Sphere — stacks={stacks}, slices={slices}\n")
        f.write(f"# Vertices: {len(vertices)}, Faces: {len(faces)}\n\n")
        
        for v in vertices:
            f.write(f"v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}\n")
        
        f.write("\n")
        
        for face in faces:
            f.write(f"f {face[0]} {face[1]} {face[2]}\n")
    
    print(f"[OK] Sphere berhasil di-generate:")
    print(f"     File    : {filepath}")
    print(f"     Stacks  : {stacks}")
    print(f"     Slices  : {slices}")
    print(f"     Radius  : {radius}")
    print(f"     Vertices: {len(vertices)}")
    print(f"     Faces   : {len(faces)}")


if __name__ == "__main__":
    # Default values
    output_path = "test/sphere.obj"
    subdivisions = 32
    
    if len(sys.argv) >= 2:
        output_path = sys.argv[1]
    if len(sys.argv) >= 3:
        try:
            subdivisions = int(sys.argv[2])
        except ValueError:
            print("[ERROR] subdivisions harus bilangan bulat.")
            sys.exit(1)
    
    generate_sphere(output_path, stacks=subdivisions, slices=subdivisions)
