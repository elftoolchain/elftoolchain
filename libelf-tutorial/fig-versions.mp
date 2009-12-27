% Show the three ELF versions in play when processing an ELF object.
%
% Copyright (c) 2006-2009 Joseph Koshy.  All rights reserved.
%
% Redistribution and use in source and binary forms, with or without
% modification, are permitted provided that the following conditions
% are met:
% 1. Redistributions of source code must retain the above copyright
%    notice, this list of conditions and the following disclaimer.
% 2. Redistributions in binary form must reproduce the above copyright
%    notice, this list of conditions and the following disclaimer in the
%    documentation and/or other materials provided with the distribution.
%
% This software is provided by Joseph Koshy ``as is'' and
% any express or implied warranties, including, but not limited to, the
% implied warranties of merchantability and fitness for a particular purpose
% are disclaimed.  in no event shall Joseph Koshy be liable
% for any direct, indirect, incidental, special, exemplary, or consequential
% damages (including, but not limited to, procurement of substitute goods
% or services; loss of use, data, or profits; or business interruption)
% however caused and on any theory of liability, whether in contract, strict
% liability, or tort (including negligence or otherwise) arising in any way
% out of the use of this software, even if advised of the possibility of
% such damage.
%
% $Id$
%
defaultfont := "Times-Roman";
prologues := 2;
u := 1cm;

input rboxes;

pair fudge;
fudge = (1.5pt,0);

verbatimtex
\documentclass{article}
\usepackage{times}
\begin{document}
etex;

def width suffix b = (b.e - b.w) enddef;

beginfig(1);

  boxjoin();

  % Draw boxes representing the ELF object, the library and the application.
  rboxit elf (btex ELF Object \raisebox{-1ex}{\small $v_2$} etex);
  rboxit library (btex ELF library \raisebox{-1ex}{\small $v_1$, $v_2$} etex);
  rboxit application (btex Application \raisebox{-1ex}{\small $v_1$} etex);

  library.e = elf.w shifted (-2u,0);
  application.e = library.w shifted (-2u,0);

  width elf = width library = width application = (3u,0);

  fixsize(elf, library, application);

  forsuffixes v = elf, library, application:
    fill bpath v withcolor .85 white;
  endfor;

  drawboxed(elf, application, library);

  % Draw `usage' links.
  path a[];
  a1 = library.w - fudge -- application.e + fudge;
  a2 = elf.w - fudge -- library.e + fudge;

  drawdblarrow a1;
  label top (btex \small $v_1$ etex, point .5 of a1);
  drawdblarrow a2;
  label top (btex \small $v_2$ etex, point .5 of a2);

endfig;
end
