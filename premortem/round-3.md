# Pre-mortem round 3

New failure modes:
1. The AU build on macOS is never validated (pluginval run only on the VST3). → R-018: plan amended — D-020
   plugin job on macOS also runs `auval -v aumu Saxa Qcod` after copying the AU to `~/Library/Audio/Plug-Ins/Components`.
