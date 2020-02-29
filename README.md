# Pathed

Pathed is a path tracer I have been working on to re-familiarize myself with C++ and solidify my understanding of rendering techniques and algorithms.

Some of Pathed features include: Lambertian, Oren-Nayar, Beckmann microfacet, Mirror, and Glass scattering models; multiple importance sampling and next event estimation; environment maps; and heterogeneous volumetric scattering.

My most recent project has been attempting to render Disney's Moana research scene. So far I've added support for B-spline curves, ptex textures, and hierarchical instancing. Currently I'm working on optimization, bug fixes, and supporting the Disney BRDF.

## Rendered Images ##
<p align="center">
<img src="https://pathed.s3-us-west-2.amazonaws.com/moana-20200217.png">
<b>Current WIP: Moana scene</b>
</p>

<p align="center">
<img src="https://pathed.s3-us-west-2.amazonaws.com/volumetric-caustic.png" width="400">
<br /><b>Volumetric caustic in homogeneous medium</b>
</p>

<p align="center">
<img src="https://pathed.s3-us-west-2.amazonaws.com/homogeneous-absorption-teapot.png" width="400">
<br /><b>Glass material, homogeneous absorption, and environment lighting</b>
</p>

<p align="center">
<img src="https://pathed.s3-us-west-2.amazonaws.com/curves-bunny.png" width="400">
<br /><b>Bunny with Bezier curve fur</b>
</p>

<p align="center">
<img src="https://pathed.s3-us-west-2.amazonaws.com/plastic--dragon.png" width="400">
<br /><b>Microfacet reflection modeled with Beckmann distribution</b>
</p>

<p align="center">
<img src="https://pathed.s3-us-west-2.amazonaws.com/mirrors-cornell.png" width="400">
<br /><b>Cornell box with mirror material</b>
</p>

<p align="center">
<img src="https://pathed.s3-us-west-2.amazonaws.com/mis-veach.png" width="400">
<br /><b>Veach MIS scene</b>
</p>

<p align="center">
<img src="https://pathed.s3-us-west-2.amazonaws.com/cornell.png" width="400">
<br /><b>Traditional Cornell box</b>
</p>
