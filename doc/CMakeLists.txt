
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
checked_adder.tex
checked_incrementer.tex
greeter.tex
multiplier.tex
scaler.tex
simple_greeter.tex
simple_incrementer.tex
simple_adder.tex
incrementer.tex
incrementer_component.tex
)

foreach(S ${LATEX_SOURCES})
  string(REPLACE ".tex" "" N ${S})
  add_custom_command(OUTPUT ${N}.svg
                     COMMAND pdflatex -halt-on-error ${CMAKE_CURRENT_SOURCE_DIR}/${N}.tex
                     COMMAND pdfcrop --margins 4 --hires ${N}.pdf
                     COMMAND pdfjoin ${N}-crop.pdf --paper a4paper --fitpaper false --outfile ${N}-resized.pdf
                     COMMAND pdfcrop --margins 4 --hires ${N}-resized.pdf
                     COMMAND pdf2svg ${N}-resized-crop.pdf ${N}.svg
                     DEPENDS
                         tikz-uml.sty
                         header.tex
                         footer.tex
                         ${N}.tex
                     )
  add_custom_target(${N}-svg ALL
                    DEPENDS ${N}.svg)
endforeach(S)

set(EXAMPLE_DIRECTORIES
hello_world
injection_scopes
multibindings
simple_injection
)

foreach(D ${EXAMPLE_DIRECTORIES})
  add_custom_command(OUTPUT ${D}-deps.svg
                     COMMAND bash < ${CMAKE_CURRENT_SOURCE_DIR}/extract_dependencies.sh > ${CMAKE_CURRENT_BINARY_DIR}/${D}.dot
                     COMMAND dot -Goverlap=prism10000 ${CMAKE_CURRENT_BINARY_DIR}/${D}.dot -Tsvg -o ${CMAKE_CURRENT_BINARY_DIR}/${D}-deps.svg
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../examples/${D}
                     DEPENDS
                        ../examples/${D}
                        extract_dependencies.sh
                     )
  add_custom_target(${D}-deps ALL
                    DEPENDS ${D}-deps.svg)
endforeach(D)

