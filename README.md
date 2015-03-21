# cl-raytrace
Toy OpenCL raytracing renderer

This is an attempt at building an OpenCL raytracing renderer. There are no plans to make this production-ready in any way.

There a a few goals this implementation strives to achieve:
- Be physically-based as much as possible. I'm trying to base this renderer on the theory presented in Physically Based Rendering: From Theory to Implementation although the implementation itself will differ drastically from pbrt's
- Implement as much as possible in OpenCL kernels. Almost nothing except OpenCL setup and input/output should happen on the host
- Be readable. I'd like this project to be a learning tool of sorts so readability and consistency trump other aspects like performance
- Be reasonably fast. I don't expect this to be a 60fps realtime renderer but not waiting a few hours for a single frame would be great.

Also, there are a few features I aim to support at some point:
- Loading and rendering of complex scenes in at least one format
- Support of triangle meshes since that's the most common type of asset
- Support for different types of materials
- Support for interfering medium affecting light propagation
- Animations and their side-effects (such as motion blur)
- Multiple camera types (pinhole, variable speed shutter, etc)
- Reconstruction of volumes and surfaces represented as particle clouds (SPH fluid simulations for example)
