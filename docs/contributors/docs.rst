Updating Documentation
======================

Overview
--------

This documentation is stored along with the source in the ``docs`` directory of the git tree and uses the `reStructuredText format <http://en.wikipedia.org/wiki/ReStructuredText>`_.

We recommend reading this `reStructuredText Primer <http://sphinx-doc.org/rest.html>`_ before editing the docs for the first time.

The documentation is automatically generated into HTML and PDF by `Read The Docs <https://readthedocs.org/>`_ once a branch has been merged into master.  This output can be found at `libAttachSQL's documentation page <http://docs.libattachsql.org/>`_.

Compiling Docs
--------------

The docs are compiled using `Sphinx Python Documentation Generator <http://sphinx-doc.org/>`_.  The libAttachSQL build system already knows how to use this.  To compile the docs please follow theses steps:

#. Install the ``python-sphinx`` package using your distribution's package manager

#. Re-run bootstrap as follows so that it picks up that Sphinx is installed::

      ./bootstrap.sh -m

#. To compile in HTML format::

      make html

There will now be an HTML version of the docs in the ``/html`` directory of the source.

Compiling PDF Docs
------------------

Sphinx required LaTeX to build PDF docs.  The following steps show you how to build PDF docs:

#. Install ``python-sphinx`` as above

#. Install the full *TeXLive* package.  In Fedora this is ``texlive-scheme-full`` and ``texlive-full`` in Ubuntu

#. Re-run bootstrap as follows so that it picks up that Sphinx and LaTeX are installed::

      ./bootstrap.sh -m

#. To compile in PDF format::

      make latexpdf

The generated PDF will be in the ``/docs/latex/`` directory.
