Usage
-----
VideoRecStation Usage
^^^^^^^^^^^^^^^^^^^^^
Operation of the VideoRecStation software is quite straightforward. You can start the program by running:

.. code-block:: bash

    VideoRecStation

This will open the programs main dialog window that contains the list of available cameras and volume monitors.

.. figure:: videorecstation.png
    :alt: VideoRecStation with two cameras
    :align: center
    :width: 100%

    VideoRecStation with two cameras.

You can activate/deactivate each camera by clicking the corresponding checkbox in the list. When a camera is activated, the program will open a window showing the live video feed from that camera. You can adjust some video settings (shutter speed, gain, white balance) for the camera by dragging the sliders in the camera window. Once you are satisfied with the video settings, you can start recording by clicking the "Start" button and stop recording by clicking the "Stop" button. The video and audio data will be saved to files in the directory specified in the configuration file (see :ref:`config-description-label` for details).

.. _scarlett-usage-label:

ALSA Scarlett Control Panel Usage
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The interface of the ALSA Scarlett Control Panel software is quite intuitive and you can probably figure most things out by just clicking around. If you nevertheless want to read some documentation, check out `the project's GitHub page <https://github.com/geoffreybennett/alsa-scarlett-gui>`_.