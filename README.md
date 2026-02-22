# About

This tool aims to fully automate not-a-zombie's vmf resizer tool. https://not-a-zombie.github.io/vmf-resizer/
It acts as a pre-processor for the vmf file before it gets compiled and adds func_detail collision brushes before
being sent to the compilers. It doesn't touch the original vmf in Hammer.

# Setup

Have Hammer++ installed: https://ficool2.github.io/HammerPlusPlus-Website/
Have tools++ installed:
https://ficool2.github.io/HammerPlusPlus-Website/tools.html
https://github.com/ficool2/misc_tools/releases/download/v1/tools_plusplus.zip

Configure Hammer++ to use tools++ (Tools->Options->Build Programs)
Add toolsplusplus.fgd to your game data. (Tools->Options->Game Confurations->Game Data->Add)

Extract anglefix.exe and vbspplusplus_wrapper.exe to where hammerplusplus.exe/vbspplusplus.exe are (bin/x64)
Rename vbspplusplus to real_vbspplusplus
Rename vbspplusplus_wrapper to vbspplusplus

# Using (basic guide for now)

For now, this guide assumes you are familiar with anglefixing and not-a-zombie's tool.
The main difference is this tool uses a face's smoothing group flags to determine which faces are the surfable 
ones, instead of specific textures. This way, we can have completely normal looking ramps in hammer.

Make a ramp and tie it to func_detail_illusionary. If you don't have this entity available, update Hammer++ and make
sure toolsplusplus.fgd is in your game data as explained above.

Select the surfable faces with the texture tool. Change the smoothing group to 32. This is equivilent of Zombie's 
"resize faces" tool. Use group 31 if you want the equivilent of "resize solids". You can set flag 30 on any face
and it will enable "debug mode" globally, which will use a visible texture rather than PLAYERCLIP on the collision.

Compile. anglefix.exe will automatically generate map-anglefixed.vmf and compile the map with it. It will also
output map-collision.vmf which only contains the collision brushes. The recommended workflow is to never put the 
collision in your actual vmf, so your hammer experience is nicer.

anglefix.exe will log it's output to the same map.log file the other compilers log to. Check it for warnings if
something isn't working.
