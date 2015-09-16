Coding Guidelines
=================
- Opening braces on same line as statement
- Tabs not spaces
- No usings at global scope in headers. 
  - Using statements in source files are fine
  - Type aliases in classes and in namespaces are fine
- In headers enums should declared as enum classes so as to avoid polluting global namespace and to avoid ambiguities
  - If an enums that can’t be enum classes (e.g. bitflags, constant values), should be contained in an enclosing scope - for example a class, struct or namespace.
- Only function definitions allowed in headers are setters and getters
  - Exceptions are source file-local classes and functor objects
- Template member functions should be defined in “.inl” files and “#include”d at the end of the header. This is purely to keep the header clean
- #include paths and all file paths should always use ‘/’ as a path separator rather than the windows specific ‘\’
  - This ensures portability as windows accepts both
  - This also avoids possible bugs involving not escaping ‘\’ in strings
- System, and library headers should be included with angle bracket syntax ‘<>’ and local headers should be included with double quote syntax

Source Control
==============
- Object files, builds and libraries should never be added to the repo
  - Rules must be added to the .gitignore for every binary file that isn’t an asset
  - If an object file is accidentally committed, the commit must be reverted and the object files must be either deleted or added to the .gitignore
    - Note that the files must also be untracked. git reset can be used to both undo the commit and untrack the files
    - Accidental commits must be undone before they are pushed
  - It is a good idea to run git status prior to committing to make sure that no unnecessary binary files have been added
- git pull should be run before each coding session begins, and if it is known that someone else is working on the project at the same time, at regular intervals (e.g. after every commit or two, or when notified of a push)
- git commit should be run after any substantial set of changes
  - Commit messages must describe changes
- git push should be run at the end of each session or at regular intervals if it is known that someone else is also working on the project

File Formats
============

Directory Structure and File Naming 
-----------------------------------
- Directories should be lowercase and should contain no spaces
- Filenames (including extensions) should all be lowercase using underscores where one would use spaces
- Source files - .cpp
- Header files - .h
- Template definition files - .inl

Asset Formats
-------------
- Models - Ogre’s .mesh format + accompanying .material files
  - Blender can export these with a plugin
- Images/Textures - .png
- Audio - .ogg
