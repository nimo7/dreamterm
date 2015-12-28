# DreamTerm

Happened to find some of my old sources for Amiga during holiday and thought in drunken stupor that it would be nice to 'immortalise' them into GitHub. So here is 20+ years old DTerm sources for the latest released version (1.50). I did minimal cleaning to make it compile.

Note that asm files use tab value of 8, C ones 2. Don't ask about coding choices, I had perfectly valid reasons back then I think :D

No intentional licensed, copyrighted, trademarked or patented functionality used in sources.

To compile (I had so much trouble to duplicate my old environment):

- install standard OS3.1 A1200/A4000 system from 199x (use cycle exact A1200/4MB Z2 fast without JIT profile for authentic compiling experience)
- install MUI3.8 usr & dev, PhxAss v4.44 and SAS/C 6.3 (later/earlier versions most likely dont work)
- optionally run dterm1.50 binary installer which will setup optional libraries, fonts etc.
- create sys:work/dterm directory and copy sources there
- do following assigns (or copy after SAS/C assigns at user-startup and reboot)

  - assign work: sys:work
  
  - assign lib:     work:dterm/2016/lib add
  - assign asminc:  include:
  - assign asminc:  work:dterm/2016/include add
  - assign include: work:dterm/2016/include add
  - assign include: sys:mui/developer/c/include add

- find AmigaOS NDK 3.9, copy content of include_h and include_i to sc:include, copy content of linker_libs to sc:lib
- execute create_catalog to make localization catalogs (uncomment last 6 lines if you installed 1.50 binary)
- smake
- set stack to 20000
- type dterm to run (optionally copy to installed dterm1.50 dir and run from there)

Guess thats all, hope you have as happy nostalgy run as I had.
