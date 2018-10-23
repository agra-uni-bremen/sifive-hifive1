%% Class CPS-Beamer: Slides for DFKI CPS.
%%
%% $Id: CPS-Beamer.cls 383 2016-03-08 15:15:39Z cxl $
%%
%% (c) DFKI. For our very generous licensing terms, please enquire within.
%%
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{CPS-Beamer}[2016/02/03 v0.1 DFKI CPS beamer slides class]


\PassOptionsToPackage{ngerman,english}{babel}
\PassOptionsToPackage{table}{xcolor}
\PassOptionsToPackage{bookmarks}{hyperref}
\DeclareOption{plain}{\PassOptionsToPackage{plain}{beamerthemeDFKI}}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{beamer}}
\ProcessOptions\relax
\LoadClass[covered]{beamer}
\mode<presentation>{\beamer@suppressreplacementstrue}

\RequirePackage[T1]{fontenc}
\RequirePackage{babel}
\RequirePackage{graphicx}
\RequirePackage{tabularx,booktabs}
\RequirePackage[scaled]{helvet}
\RequirePackage{tikz}

\usetheme{DFKI}

\mode
<all>
