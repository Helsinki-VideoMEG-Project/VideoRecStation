File Format Description
-----------------------

Audio and video is stored in ``*.aud`` and ``*.vid`` files; if there are multiple video cameras, each camera's video is stored in a separate ``*.vid`` file. The file content is described below in the order of it's appearance in the file. The general structure of the file is:
  #. Magic string. The string is ``HELSINKI_VIDEO_MEG_PROJECT_VIDEO_FILE`` for video files and ``HELSINKI_VIDEO_MEG_PROJECT_AUDIO_FILE`` for audio files.
  #. Format version number. This is a 32 bit integer specifying the format version.

Format of the rest of the file is version-dependent.  

Version 0
^^^^^^^^^
The audio file additionally contains: 
  #. A 32-bit integer describing the sampling rate in Hz.  
  #. A 32-bit integer describing the number of channels.  
  #. Two bytes specifying the data format for a single audio sample. The two bytes should be interpreted as an Python `format string <https://docs.python.org/3/library/struct.html#struct-format-strings>`_ in ascii encoding. For example, ``<i`` means little-endian signed four-byte integer.

The rest of the both types of files is filled with data blocks. Each data block consists of:  
  #. 64-bit integer containing the block's timestamp. The timestamp is the number of milliseconds since the UNIX epoch.  
  #. 32-bit integer containing the length of the payload in bytes.  
  #. The payload:

    - For audio files the payload is the raw sample data. The channels are interleaved. For audio, all the blocks are of the same length. The timestamp corresponds to the first sample in the block.
    - For video the payload is a single frame compressed in jpeg format. If you save the payload as a separate file you will get a normal jpeg file that you can open in an image viewer, etc.

Other versions
^^^^^^^^^^^^^^
Currently only version 0 is implemented.
