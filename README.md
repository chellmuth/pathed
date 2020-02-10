# Pathed

Pathed is a path tracer I have been working on to re-familiarize myself with C++ and solidify my understanding of rendering techniques and algorithms.

Some of Pathed features include: Lambertian, Oren-Nayar, Beckmann microfacet, Mirror, and Glass scattering models; multiple importance sampling and next event estimation; environment maps; and heterogeneous volumetric scattering.

My most recent endeavor has been attempting to render Disney's Moana research scene. So far I've added support for B-spline curves, ptex textures, and hierarchical instancing. Currently I'm working on optimization, bug fixes, and supporting the Disney BRDF.

## Rendered Images ##
<p align="center">
<img src="https://pathed.s3-us-west-2.amazonaws.com/moana-wip--shotCam.png">
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
<br /><b>Bunny with fur modeled as Bezier curves</b>
</p>

<p align="center">
<img src="https://pathed.s3-us-west-2.amazonaws.com/glass-and-mirror.png?x=1" width="400">
<br /><b>Cornell box with glass and mirror material</b>
</p>

<p align="center">
<img src="https://pathed.s3-us-west-2.amazonaws.com/mis-veach.png" width="400">
<br /><b>Veach MIS scene</b>
</p>

<p align="center">
<img src="https://pathed.s3-us-west-2.amazonaws.com/cornell.png" width="400">
<br /><b>Traditional Cornell box</b>
</p>

[moana-shotCam]: https://pathed.s3-us-west-2.amazonaws.com/moana-wip--shotCam.png "Moana shotCam WIP"
[teapot]: https://pathed.s3-us-west-2.amazonaws.com/homogeneous-absorption-teapot.png "Homogeneous absorption teapot"
[curves]: https://pathed.s3-us-west-2.amazonaws.com/curves-bunny.png "Bunny with Bezier-curved fur"
[volumetric-caustic]: https://pathed.s3-us-west-2.amazonaws.com/volumetric-caustic.png "Volumetric caustic from scattering media"
[mirrors]: https://pathed.s3-us-west-2.amazonaws.com/mirrors-cornell.png "Mirrored Cornell box"
[mis]: https://pathed.s3-us-west-2.amazonaws.com/mis-veach.png "Veach MIS scene"
[cornell]: https://pathed.s3-us-west-2.amazonaws.com/cornell.png "Classic Cornell box scene"
[microfacet]: https://pathed.s3-us-west-2.amazonaws.com/microfacet--dragon.png "Beckmann microfacet"
