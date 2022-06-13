# TacticalVRCorePlugin
Unreal Engine 4 (and in the future probably 5) plugin for implementing a VR Shooter. Follow the instructions to setup configs, etc accordingly to be able to use this plugin.

# Example Project
A functional example project can be found in the following repository: [Tomura / TacticalVRCoreExample](https://github.com/Tomura/TacticalVRCoreExample).

# Branches
In generally it is recommended to use the 'main' branch. The 'development' branch will contain live updates I do during development of my mainline project, which one can see on my youtube channel. It is usually a lot more unstable, but contains the newest in-development features. At the moment I do not follow a process such as git-flow, because it is a solo project. 

# Installation
Put the plugin into your projects Plugin directory and activate it via the editor. Please make sure to read the wiki, about the necessary config options, or alternatively modify the plugin source code to fit your configuration.

Make sure to use the following:
- Unreal Engine 4.27.*
- OpenXR
- VRExpansionPlugin

Force Tube VR is not recommended anymore. As a replacement a GunHapticsComponent was introduced. Please override it or create a Blueprint from it, and implement your ForceTube integration there.
The wiki contains a simple example: [Wiki - ForceTubeVR Integration](https://github.com/Tomura/TacticalVRCorePlugin/wiki/ForceTubeVR-Integration)

# Contributions
I appreciate contributions via pull requests. This plugin is developed in conjunction with my own hobby VR project, so please understand that I may reject pull requests, that do not fit into my overall vision. Even little things helps, such as writing additional docstrings, etc.

