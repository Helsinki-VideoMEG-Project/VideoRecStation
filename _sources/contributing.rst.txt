Contributing to the Project
---------------------------
If you want to contribute to the project, feel free to send a PR to `Andrey Zhdanov <https://github.com/andreyzhd>`_.

Coding conventions
^^^^^^^^^^^^^^^^^^
Since VideoRecStation is built on Qt, in general, try to follow Qt's `Coding Conventions <https://wiki.qt.io/Coding_Conventions>`_, and in particular `Qt Coding Style <https://wiki.qt.io/Qt_Coding_Style>`_ whenever it makes sense.

Building the documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^
The documentation for the project is written in `reStructuredText <https://docutils.sourceforge.io/rst.html>`_ and is compiled into html using `Sphinx <https://www.sphinx-doc.org>`_ and `Furo <https://github.com/pradyunsg/furo>`_ theme. To edit the documentation, modify the sources in the ``docs/src`` folder. To buld the html files, install Sphinx and Furo on you computer, then from the ``docs`` folder run:

.. code-block:: bash

    sphinx-build -M html src html

.. note::

    If you have conda installed on your machine, you can install Sphinx and Furo into a new conda environment named ``sphinx`` by running:

    .. code-block:: bash

        conda create -n sphinx sphinx furo -c conda-forge