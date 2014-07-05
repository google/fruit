
add_custom_command(OUTPUT tikz-uml.sty
                   COMMAND wget http://perso.ensta-paristech.fr/~kielbasi/tikzuml/src/tikzuml-v1.0b-2013-02-01.tbz
                   COMMAND tar xf tikzuml-v1.0b-2013-02-01.tbz
                   COMMAND mv tikzuml-v1.0b/tikz-uml.sty tikz-uml.sty
                   )
add_custom_command(OUTPUT header.tex footer.tex
                   COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/common-header.tex header.tex
                   COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/common-footer.tex footer.tex
                   DEPENDS 
                       ${CMAKE_CURRENT_SOURCE_DIR}/common-header.tex
                       ${CMAKE_CURRENT_SOURCE_DIR}/common-footer.tex
                   )

set(LATEX_SOURCES
greeter.tex
simple_greeter.tex
)

foreach(S ${LATEX_SOURCES})
  string(REPLACE ".tex" "" N ${S})
  add_custom_command(OUTPUT ${N}.pdf
                     COMMAND pdflatex -halt-on-error ${CMAKE_CURRENT_SOURCE_DIR}/${N}.tex
                     COMMAND pdfcrop --resolution 300 --margins 4 --hires ${N}.pdf
                     COMMAND pdfjoin ${N}-crop.pdf --paper a4paper --fitpaper false --outfile ${N}-resized.pdf
                     COMMAND pdfcrop --resolution 300 --margins 4 --hires ${N}-resized.pdf
                     COMMAND pdf2svg ${N}-resized-crop.pdf ${N}.svg
                     DEPENDS
                         tikz-uml.sty
                         header.tex
                         footer.tex
                         ${N}.tex
                     )
  add_custom_target(${N} ALL
                    DEPENDS ${N}.pdf)
endforeach(S)
