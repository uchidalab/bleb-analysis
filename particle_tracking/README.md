# Particle Tracking
## System requirements
The system requirements are described below.
- Ubuntu 16.04 LTS
- Fiji ImageJ 2.0.0-rc-68/1.52d; Java 1.8.0_66 [64bit]
- Python 3.5.2
- Several Python libraries
    - numpy==1.14.5
    - opencv-python==3.4.2
    - matplotlib==2.2.3
    - pillow==5.2.0

## Installation guide
Installation procedure are described below.
1. Install Fiji ImageJ 2.0.0-rc-68/1.52d.
1. Install Python 3.5.2 and Several libraries.

The above installation is completed in abaut an hour.

## Instructions
Instruction procedure are described below.

1. Running "programs/LoGfilter.py" with Python.
1. Running "programs/findmaxima.py" with Python in Fiji ImageJ.
    - Lanching Fiji ImageJ, you will see the main window.
    - Opening the Python script file, be it via [File > Open] in menu bar.
    - Running the Python script, be it via [Run > Run] in menu bar.
    - When the setting window opened, you should set some arguments.
        - Input directory: specify the absolute path to the "LoG" directory,
         like "/home/user/particle_tracking/output/LoG".
        - Output directory: specify the absolute path to the output directory
         for result of findmaxima, like "/hone/user/output".
        - File extension: require string "png".
        - Noise tolerance: require string "100".
    - When you completed setting arguments, click "OK" button and running
     python script.
1. Running "programs/main.py" with Python.

The above process is completed in about an hour.
The expected output is in the "expected_output" directory.

This software is released under the MIT License, see "LICENSE.txt".