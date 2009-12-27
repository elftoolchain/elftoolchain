% Show the basic layout of an ELF file.
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
defaultfont:= "Times-Roman";
prologues:= 2;
u := 2cm;

input boxes

arrowoff = 2pt;						% offset for arrows
arrowangle = 135;

pair boxht, boxwd;

boxwd = (u,0);
boxht = (0,.5u);

def elflabel (expr p, off) (suffix pos, d) text $ =
  begingroup
    save n,q,wd,fudge,da; pair n; picture q;
    n = p + off; q = $ infont defaultfont;
    wd = xpart (lrcorner q - llcorner q) + labeloffset;	% width of underline
    label pos (q, n);
    if d = up:
      da = arrowangle; fudge = -arrowoff;
    else:
      da = -arrowangle; fudge = arrowoff;
    fi;
    drawarrow (n + (wd,0)) -- n {left} .. {dir da} (p + (0,fudge));
  endgroup
enddef;

def elfsegment (suffix b)(expr wd) text $ =
  boxit b ($);
  b.ne - b.nw = wd;
enddef;

beginfig(1);

  % Draw the ELF file's layout.
  boxjoin(a.ne=b.nw; a.se=b.sw);

  elfsegment (ehdr, boxwd) "EHDR";
  elfsegment (bl0, .1 boxwd);
  elfsegment (phdr, 1.2 boxwd) "PHDR";
  elfsegment (bl1, .1 boxwd);
  elfsegment (scnd, 2 boxwd) "SECTION DATA";
  elfsegment (bl2, .1 boxwd);
  elfsegment (shdr, 1.2 boxwd) " SHDR ";

  fill bpath ehdr withcolor .5 white;
  fill bpath phdr withcolor .9 white;
  fill bpath scnd withcolor .7 white;
  fill bpath shdr withcolor .7 white;

  drawboxed(ehdr,bl0,phdr,bl1,scnd,bl2,shdr);

  % Draw labels.
  elflabel (ehdr.n, (.5u,.75u), urt, down) "ELF Executable Header";
  elflabel (scnd.n, (.5u,.75u),  urt, down) "ELF Section Contents";
  elflabel (phdr.s, (.5u,-.75u), urt, up) "ELF Program Header Table";
  elflabel (shdr.s, (.5u,-.75u), urt, up) "ELF Section Header Table";

endfig;
end
