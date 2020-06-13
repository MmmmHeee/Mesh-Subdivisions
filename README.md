# CG2 Project 5 - Subdivision Surfaces

## Introduction

This is the final project of ShanghaiTech University's Computer Graphics II(CS271) course.
We implemented 3 mesh subdivision algorithms: Loop, Catmull-Clark and Doo-Sabin. Those algorithms are only performed 
on the position of vertices. In theory, normals and uvs can be modified in the same way to form a complete subdivison algorithm.

Under the instruction of Prof. ChiHan Peng, we successfully implemented the whole framework to perform subdivision algorithms.

## Contributor

[MmmmHeee](https://github.com/MmmmHeee): Loop Subdivision

[RoarlisF/KP31](https://github.com/SuomiKP31): Catmull-Clark Subdivision

[Qihao Zhang](https://github.com/zhaqu): Doo-Sabin Subdivision

## Result

Here are some gifs showcasing 1-3 iterations of these algorithms. We used MeshLab for visualization.

Loop Subdivision:

![avatar](presentation/result_gif/loop_cube_tri.gif)
![avatar](presentation/result_gif/loop_tetrahedron.gif)

Catmull-Clark Subdivision:

![avatar](presentation/result_gif/catmull_cube_quad.gif)
![avatar](presentation/result_gif/catmull_tetrahedron.gif)

Doo-Sabin Subdivision:

![avatar](presentation/result_gif/doosabin_cube_quad.gif)
![avatar](presentation/result_gif/doosabin_tetrahedron.gif)
