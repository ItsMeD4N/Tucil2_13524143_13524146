# Tucil2_13524143_13524145
## Voxelization 3D dengan Octree & Divide and Conquer

Program ini menerima file model 3D (`.obj`) dan mengubahnya menjadi representasi **voxel** menggunakan algoritma **Divide and Conquer** berbasis struktur data **Octree**. Setiap voxel dihasilkan dari leaf node Octree pada kedalaman maksimum yang berpotongan dengan permukaan mesh.

Deteksi tabrakan antara AABB (sub-kubus) dan segitiga mesh menggunakan **Separating Axis Theorem (SAT)** berdasarkan paper Akenine-Möller (2001), yang menguji 13 sumbu pemisah secara efisien.

Output berupa file `.obj` baru berisi kubus-kubus voxel yang setiap sisinya sudah di-**triangulasi** (1 voxel = 6 sisi = 12 segitiga).

### Fitur Utama
- **Parser `.obj` yang kebal** — mendukung semua format face (`f v`, `f v/vt`, `f v/vt/vn`, `f v//vn`), indeks negatif, dan mengabaikan baris non-vertex/face.
- **SAT 13-axis** — deteksi tabrakan AABB vs Triangle yang akurat dan cepat.
- **Multithreading** — paralelisasi 8 cabang Octree di depth awal menggunakan `std::async` untuk mempercepat komputasi.
- **Memory-safe** — menggunakan `std::unique_ptr` untuk manajemen memori Octree, `std::atomic` dan `std::mutex` untuk thread-safety.

---

## Requirement

| Komponen | Versi Minimum |
|----------|--------------|
| Compiler | g++ 8+ (mendukung C++17) |
| Standard | C++17 (`<filesystem>`, `<future>`, `<atomic>`) |
| OS | Windows / Linux / macOS |

---

## Cara Kompilasi

### Windows (MinGW)
```bash
g++ -std=c++17 -O2 -static -o voxelizer src/main.cpp
```
> **Catatan:** Flag `-static` wajib di MinGW Windows untuk menghindari error DLL entrypoint (`STATUS_ENTRYPOINT_NOT_FOUND`).

### Linux / macOS
```bash
g++ -std=c++17 -O2 -pthread -o voxelizer src/main.cpp
```

---

## Cara Menjalankan

```bash
./voxelizer <path/ke/model.obj> <maxDepth>
```

### Contoh
```bash
# Voxelisasi kubus dengan kedalaman 3
./voxelizer test/cube.obj 3

# Voxelisasi bola dengan kedalaman 5
./voxelizer test/sphere.obj 5
```

### Contoh Output CLI
```
========================================
  VOXELIZATION - Octree Divide & Conquer
========================================
Input        : test/cube.obj
Max Depth    : 3
Vertices     : 8
Triangles    : 12
----------------------------------------
Memproses...

========================================
          HASIL VOXELISASI
========================================
Jumlah Voxel       : 296
Jumlah Vertex Out  : 2368
Jumlah Face Out    : 3552
----------------------------------------
Nodes Terbentuk per Depth:
  Depth 0 : 1
  Depth 1 : 8
  Depth 2 : 64
  Depth 3 : 448
Nodes Dipangkas (Pruned) per Depth:
  Depth 0 : 0
  Depth 1 : 0
  Depth 2 : 8
  Depth 3 : 152
----------------------------------------
Max Depth          : 3
Waktu Eksekusi     : 0.002056 detik
Output File        : test/output_voxelized.obj
========================================
```

---

## Struktur Proyek

```
Tucil2_13524143_13524146/
├── src/
│   ├── main.cpp          # Entry point, CLI, statistik
│   ├── types.hpp         # Struct: Vector3, Triangle, AABB, VoxelStats
│   ├── parser.hpp        # Parser & exporter file .obj
│   └── octree.hpp        # Octree, SAT, Divide & Conquer, multithreading
├── test/
│   ├── cube.obj          # Model test (kubus)
│   └── output_voxelized.obj  # Hasil voxelisasi (auto-generated)
├── tools/
│   └── generate_sphere.py    # Generator model bola untuk stress test
└── README.md
```

---

## Author

| NIM | Nama |
|-----|------|
| 13524143 | Daniel Putra Rywandi S. |
| 13524145 | Dzakwan Muhammad Khairan Putra Purnama |
