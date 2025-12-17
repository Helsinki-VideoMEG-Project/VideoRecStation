Installation Instructions
-------------------------

Prerequisites
^^^^^^^^^^^^^
VideoRecStation runs on Ubuntu 24.04 LTS (Jammy Jellyfish) on amd64 architecture. It might work on other versions of Ubuntu or other Linux distributions, but this has not been tested.

Install 3-rd party components
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Before you can use VideoRecStation, you need to install VimbaX, GPUJPEG and ALSA Scarlett Control Panel (alsa-scarlett-gui). The latter is, strictly speaking, optional, but it provides a convenient GUI for controlling the audio settings of your Scarlett interface.

Install VimbaX
++++++++++++++
VimbaX is a software package for controlling USB cameras made by Allied Vision Technologies. You can download it from the `Allied Vision website <https://www.alliedvision.com/en/products/software/vimba-x-sdk/>`_. Assume you have downloaded it to ``~/Downloads/VimbaX_Setup-2025-3-Linux64.tar.gz``, and want to install it to ``/opt/VimbaX_2025-3``. You can do this by running the following commands:

.. code-block:: bash

   sudo mkdir /opt/VimbaX_2025-3
   sudo tar -xvzf ~/Downloads/VimbaX_Setup-2025-3-Linux64.tar.gz -C /opt/VimbaX_2025-3 --strip-components=1

Remove the camera simulators that come with VimbaX, so that they don't clutter the list of available cameras:

.. code-block:: bash

   sudo rm -f /opt/VimbaX_2025-3/cti/*Camera_Simulator*

Now you need to install VimbaX transport layers. Run the following commands:

.. code-block:: bash

   cd /opt/VimbaX_2025-3/cti
   sudo ./Install_GenTL_Path.sh

and reboot your system. You can now test the VimbaX by connecting an Alvium USB camera to your computer and running:

.. code-block:: bash

   /opt/VimbaX_2025-3/bin/VimbaXViewer

The Vimba X Viewer window should appear and you should see the camera in the list of detected cameras.

Finally, add the VimbaX library path to your system's library path. You can do this by creating a new file in the ``/etc/ld.so.conf.d/`` directory:

.. code-block:: bash

   echo "/opt/VimbaX_2025-3/api/lib" | sudo tee /etc/ld.so.conf.d/vimbax.conf

Then, update the linker cache:

.. code-block:: bash

   sudo ldconfig

Install GPUJPEG library
+++++++++++++++++++++++

`GPUJPEG <https://github.com/CESNET/GPUJPEG>`_ is a library for JPEG encoding and decoding using NVIDIA (and, possibly, other) GPUs. To install it, download the latest release from the `GPUJPEG releases page <https://github.com/CESNET/GPUJPEG/releases>`_. Assume you have downloaded the file ``GPUJPEG.CI.Linux.build.all.CUDA.architetures.zip`` to ``~/Downloads``. Install it by running the following commands:

.. code-block:: bash

   sudo unzip ~/Downloads/GPUJPEG.CI.Linux.build.all.CUDA.architetures.zip -d /opt/GPUJPEG
   echo "/opt/GPUJPEG/lib" | sudo tee /etc/ld.so.conf.d/gpujpeg.conf
   sudo ldconfig

.. note::

   1. Make sure that you have the NVIDIA drivers and CUDA toolkit installed on your system.
   2. The above instructions were tested with GPUJPEG version 0.27.8 in Dec 2025.

Install ALSA Scarlett Control Panel
+++++++++++++++++++++++++++++++++++
The ALSA Scarlett Control Panel (alsa-scarlett-gui) is a graphical user interface for controlling the audio settings of your Scarlett audio interface box. You can install it by following the `instructions <https://github.com/geoffreybennett/alsa-scarlett-gui/blob/master/docs/INSTALL.md>`_ from the project's `GitHub page <https://github.com/geoffreybennett/alsa-scarlett-gui>`_. In a nutshell:

Install the packages needed to build the software:

.. code-block:: bash

   sudo apt -y install git make gcc libgtk-4-dev libasound2-dev libssl-dev

Clone the repository:

.. code-block:: bash

   git clone https://github.com/geoffreybennett/alsa-scarlett-gui.git

Build and install the software:

.. code-block:: bash

   cd alsa-scarlett-gui/src
   make -j$(nproc)
   sudo make install

You now should be able to run ALSA Scarlett Control Panel from the Ubuntu applications menu.

.. note::

   The installation instructions for VimbaX and ALSA Scarlett Control Panel were tested in Aug 2025. As these are provided by third parties, there are no guarantees that the instructions will work in the future (or that these components will be available at all for that matter). If you have problems with installing VimbaX and/or ALSA Scarlett Control Panel, try checking their respective web pages.

Configure USB-related kernel parameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
To avoid potential problems when using VideoRecStation you need to correctly set a few Linux kernel parameters related to USB:
    1. Full Speed USB (USBFS) buffer size.
    2. USB autosuspend settings.
    
By default, the USBFS buffer size is set to 16MB, which might not be enough, especially when using multiple cameras. You can check the current buffer size by running:

.. code-block:: bash

   cat /sys/module/usbcore/parameters/usbfs_memory_mb

Similarly, USB autosuspend is enabled by default, which might cause problems with VideoRecStation setup. You can check the current autosuspend setting by running:

.. code-block:: bash

   cat /sys/module/usbcore/parameters/autosuspend

If the output of the above command is not ``-1``, USB autosuspend is enabled.

To increase the buffer size permanently to 1000MB (which is probably sufficient for most use cases) and enable USB autosuspend, add the kernel parameter ``usbcore.usbfs_memory_mb=1000 usbcore.autosuspend=-1`` to the bootloader configuration. To do this you have to edit the GRUB configuration file ``/etc/default/grub``. Find the line that looks like this:

.. code-block:: bash

   GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"

and add ``usbcore.usbfs_memory_mb=1000`` to it, so that it looks like this:

.. code-block:: bash

   GRUB_CMDLINE_LINUX_DEFAULT="quiet splash usbcore.usbfs_memory_mb=1000 usbcore.autosuspend=-1"

After that, update the GRUB configuration by running:

.. code-block:: bash

   sudo update-grub

And reboot your system. After rebooting, running

.. code-block:: bash

   cat /sys/module/usbcore/parameters/usbfs_memory_mb

should show 1000, and running

.. code-block:: bash

   cat /sys/module/usbcore/parameters/autosuspend

should show -1.

.. note::

   You can temporarily (until next reboot) change the USBFS buffer size by running and autosuspend setting by running the following commands:

   .. code-block:: bash

      sudo sh -c 'echo 1000 > /sys/module/usbcore/parameters/usbfs_memory_mb'
      sudo sh -c 'echo -1 > /sys/module/usbcore/parameters/autosuspend'



Set up exclusive access by VideoRecStation to the audio device
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. note:: 

   Currently the VideoRecStation is designed to work with a Focusrite Scarlett 18i20 [3rd Gen] audio interface. You might be able to make it work with somehing else by hacking the config file, but to add proper support for other audio interfaces you need to modify the source code (and obviously recompile it).

In Ubuntu 24.04 LTS, the audio devices are managed by a system service called PipeWire. VideoRecStation needs exclusive access to the audio device, so to avoid conflicts with PipeWire, you need to configure PipeWire to ignore the Scarlett audio interface. To do this, first find the id of your Scarlett box. Make sure that the Scarlett box is connected to your computer and turned on, then run:

.. code-block:: bash

   pactl list short cards

Look for a line that contains the id of your Scarlett box. The id should look something like ``alsa_card.usb-Focusrite_Scarlett_18i20_USB_12345678-00``. Write down the id.

.. note:: 

   You can also use ``wpctl status`` and ``wpctl inspect <ID>`` to get the same id. Look for the line like ``device.name = "alsa_card.usb-Focusrite_Scarlett_18i20_USB_12345678-00"``.

Next we need to create a WirePlumber rule to disable the device. Create a new file ``~/.config/wireplumber/main.lua.d/99-disable-scarlett.lua`` with the following content (replace ``alsa_card.usb-Focusrite_Scarlett_18i20_USB_12345678-00`` with the id you found above):

.. code-block:: lua

   rule = {
      matches = {
         {
            { "device.name", "equals", "alsa_card.usb-Focusrite_Scarlett_18i20_USB_12345678-00" },
         },
      },
      apply_properties = {
         ["device.disabled"] = true,
      },
   }

   table.insert(alsa_monitor.rules, rule)

Then restart WirePlumber by running:

.. code-block:: bash

   systemctl --user restart wireplumber

You can check that the PipeWire is no longer managing the Scarlett box by running:

.. code-block:: bash

   pactl list short cards

The Scarlett box should no longer be listed.

.. note:: 

   The above instructions work for WirePlumber versions 0.4.x (which was the default in Ubuntu 24.04 LTS as of Aug 2025) Between versions 0.4.x and 0.5.x the format of the configuration files for WirePlumber changed. If you are using WirePlumber 0.5.x or later, instead of ``~/.config/wireplumber/main.lua.d/99-disable-scarlett.lua`` you need to create the file ``~/.config/wireplumber/wireplumber.conf.d/99-disable-scarlett.conf`` with the following content (again, replace ``alsa_card.usb-Focusrite_Scarlett_18i20_USB_12345678-00`` with the id you found above):

   .. code-block::

      monitor.alsa.rules = [
         {
            matches = [
               { device.name = "alsa_card.usb-Focusrite_Scarlett_18i20_USB_12345678-00" }
            ]
            actions = {
               update-props = {
                  device.disabled = true
               }
            }
         }
      ]

   You can check the version of WirePlumber by running:

   .. code-block:: bash

      wireplumber --version

Install the VideoRecStation software
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build and install VideoRecStation binary
++++++++++++++++++++++++++++++++++++++++
Install the packages needed to build VideoRecStation:

.. code-block:: bash

   sudo apt -y install qt6-base-dev designer-qt6 libasound2-dev g++

Clone the repository:

.. code-block:: bash

   git clone https://github.com/Helsinki-VideoMEG-Project/VideoRecStation.git

Modify the ``INCLUDEPATH`` and ``LIBS`` sections of the file ``src/VideoRecStation.pro`` to reflect the location where you have installed VimbaX (e.g., ``/opt/VimbaX_2025-3/api/include`` and ``/opt/VimbaX_2025-3/api/lib`` if you installed VimbaX in ``/opt/VimbaX_2025-3``) and GPUJPEG (e.g., ``/opt/GPUJPEG/include`` and ``/opt/GPUJPEG/lib`` if you installed GPUJPEG in ``/opt/GPUJPEG``).

Build and install the software:

.. code-block:: bash

   cd VideoRecStation
   qmake6 src/VideoRecStation.pro
   make -f Makefile.Release
   sudo cp VideoRecStation /usr/local/bin/

You now should be able to run VideoRecStation from the command line by typing:

.. code-block:: bash

   VideoRecStation

Configure VideoRecStation
+++++++++++++++++++++++++
When you run VideoRecStation for the first time, it will create a configuration file ``~/.config/Helsinki VideoMEG Project/VideoRecStation.conf`` with default settings. You should edit this file to set the parameters according to your setup. In particular, you should modify the sound card configuration and the storage path.

.. _configure-audio-settings-label:

Configure audio settings
""""""""""""""""""""""""
As mentioned above, we assume you want to use a Focusrite Scarlett 18i20 [3rd Gen] audio interface with the VideoRecStation. For that we need to find the ALSA device name through which Linux can access the Scarlett box. First, connect the Scarlett interface to your computer and turn it on. Next run:

.. code-block:: bash

   arecord -L | grep -A1 "^hw:"

This should provide an output that looks something like this:

.. code-block:: bash

   hw:CARD=sofhdadsp,DEV=0
       sof-hda-dsp, 
   --
   hw:CARD=sofhdadsp,DEV=6
       sof-hda-dsp, 
   --
   hw:CARD=sofhdadsp,DEV=7
       sof-hda-dsp, 
   --
   hw:CARD=USB,DEV=0
       Scarlett 18i20 USB, USB Audio

Note the line ``Scarlett 18i20 USB, USB Audio``. The line above it reads ``hw:CARD=USB,DEV=0``. In this case the ALSA device name that we are looking for is ``hw:USB,0`` (note that it might be something different on your system).

Assuming that the device name is ``hw:USB,0``, modify the ``[audio]`` section of the file ``~/.config/Helsinki VideoMEG Project/VideoRecStation.conf`` as follows:

.. code-block:: ini

   [audio]
   frames_per_period=940
   input_audio_device="hw:USB,0"
   num_periods=10
   output_audio_device="hw:USB,0"
   sampling_rate=44100
   speaker_buffer_size=4
   use_speaker_feedback=true

Configure data storage path
"""""""""""""""""""""""""""
Create a folder where you want to store the recorded video and audio data and make sure that the user running VideoRecStation has write permissions to it. Then modify the ``data_storage_path`` parameter in the ``[misc]`` section of the file ``~/.config/Helsinki VideoMEG Project/VideoRecStation.conf`` to poit to your folder. For example, if you want to store the data in ``/data/videomeg``, make sure that the configuration file contains the following:

.. code-block:: ini

   [misc]
   data_storage_path=/data/videomeg

.. note::

   Make sure that VideoRecStation is not running when you are editing the configuration file.