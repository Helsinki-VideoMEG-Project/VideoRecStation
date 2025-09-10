Configuration File Description
------------------------------
VideoRecStation stores configuration in ``~/.config/Helsinki VideoMEG Project/VideoRecStation.conf`` in INI format. Below is a partial description of the configuration options. The default configuration file is automatically created when you run VideoRecStation for the first time.

.. note:: 

    VideoRecStation loads the configuration file on the startup and saves it back on exit. If you edit the configuration file manually while VideoRecStation is running, your changes will likely be overwritten when you exit VideoRecStation.

[audio] section
^^^^^^^^^^^^^^^
    * ``input_audio_device`` - a string specifying ALSA audio device to use for input.
    * ``output_audio_device`` - a string specifying ALSA audio device to use for output.

    Typically, ``input_audio_device`` and ``output_audio_device`` should be set to the same ALSA device name (e.g. ``hw:USB,0``). Normally, this should be the name of your Focusrite Scarlett 18i20 [3rd Gen] audio interface, that you find with the command ``arecord -l`` (see :ref:`configure-audio-settings-label` for details). If you need to run VideoRecStation without the Scarlett box, you can set these options to ``default`` (this is mostly useful for testing purposes).

    * ``sampling_rate`` - an integer specifying the audio sampling rate in Hz.
    * ``frames_per_period`` - an integer specifying the number of audio frames (samples) per period (chunk of audio data that the sound card driver processes at once).
    * ``num_periods`` - an integer specifying the number of periods in the audio buffer.

    You should set ``sampling_rate``, ``frames_per_period``, and ``num_periods`` to the values that your audio interface supports, otherwise VideoRecStation will not work. For the Focusrite Scarlett 18i20 [3rd Gen], the correct values are listed in :ref:`configure-audio-settings-label`.

    * ``use_speaker_feedback`` - a boolean (``true`` or ``false``) specifying whether VideoRecStation should play back the audio input through the audio output (``true``) or not (``false``).
    * ``speaker_buffer_size`` - an integer specifying the size of the audio buffer used for playback through the audio output. Larger values will increase the delay of the audio playback. A reasonable value for this parameter is 4. This option is ignored if ``use_speaker_feedback`` is set to ``false``.

    ``use_speaker_feedback`` and ``speaker_buffer_size`` are useful for introducing a controlled delay into the audio, for example to compensate for audio-video synchronization issues. Note that you can also set up the Scarlett box to achieve a similar effect (by using the Scarlett Control Panel, for instructions see :ref:`scarlett-usage-label`), albeit without the delay.

[video] section
^^^^^^^^^^^^^^^
    * ``color`` - a boolean (``true`` or ``false``) specifying whether to record color video (``true``) or grayscale video (``false``).
    * ``jpeg_quality`` - an integer from 0 to 100 specifying the JPEG quality for video compression (100 means best quality and least compression, 0 means worst quality and most compression). 80 is probably a reasonable value.
    * ``height``, ``width``, ``offset_x``, ``offset_y`` - integers specifying the resolution and cropping of the recorded video. The recorded video will have size ``width`` x ``height`` pixels, and will be cropped from the original camera image starting at pixel (``offset_x``, ``offset_y``).
    * ``use_external_trigger`` - a boolean (``true`` or ``false``) specifying whether to use an external trigger for acquisition of each frame.
    * ``external_trigger_source`` - a string specifying the name of a trigger line (e.g. ``Line0``) to use as the source of the external trigger. This option ignored if ``use_external_trigger`` is set to ``false``.

.. _config-description-label:

[misc] section
^^^^^^^^^^^^^^
  * ``data_storage_path`` - a string specifying the path to the directory where recorded data (video and audio files) will be stored.