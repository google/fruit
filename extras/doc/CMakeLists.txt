

add_custom_command(OUTPUT tikz-uml.sty
                   COMMAND wget http://perso.ensta-paristech.fr/~kielbasi/tikzuml/var/files/src/tikzuml-v1.0-2016-03-29.tbz
                   COMMAND tar xf tikzuml-v1.0-2016-03-29.tbz
                   COMMAND mv tikzuml-v1.0-2016-03-29/tikz-uml.sty tikz-uml.sty
                   )
add_custom_command(OUTPUT header.tex footer.tex
                   COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/common-header.tex header.tex
                   COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/common-footer.tex footer.tex
                   DEPENDS 
                       ${CMAKE_CURRENT_SOURCE_DIR}/common-header.tex
                       ${CMAKE_CURRENT_SOURCE_DIR}/common-footer.tex
                   )

set(LATEX_SOURCES
bar_handler.tex
bind.tex
bind_instance.tex
cached_greeter.tex
cached_greeter_test.tex
car_component.tex
checked_adder.tex
checked_incrementer.tex
component_composition.tex
component_dep_loop.tex
foo_handler.tex
greeter.tex
incrementer.tex
incrementer_component.tex
inject_macro.tex
inject_macro_no_args.tex
inject_macro_template.tex
inject_typedef_greeter.tex
inject_typedef_writer.tex
inject_typedef_writer2.tex
inject_typedef_templated_constructor.tex
multiplier.tex
parametrized_component.tex
provider.tex
provider_functor.tex
register_constructor.tex
register_constructor_component.tex
register_factory.tex
register_factory_use.tex
register_factory_macro.tex
request_dispatcher.tex
request_injector.tex
scaler.tex
server.tex
simple_greeter.tex
simple_incrementer.tex
simple_adder.tex
templated_component.tex
)

foreach(S ${LATEX_SOURCES})
  string(REPLACE ".tex" "" N ${S})
  add_custom_command(OUTPUT ${N}.png
                     COMMAND pdflatex -halt-on-error ${CMAKE_CURRENT_SOURCE_DIR}/${N}.tex
                     COMMAND convert -density 300 -trim ${N}.pdf -quality 100 -sharpen 0x1.0 ${N}.png
                     # This normalizes the PNG files, so that we avoid tracking multiple copies of the same file in the Github wiki repo.
                     COMMAND exiftool -all= -overwrite_original ${N}.png
                     DEPENDS
                         tikz-uml.sty
                         header.tex
                         footer.tex
                         ${N}.tex
                     )
  add_custom_target(${N}-png ALL
                    DEPENDS ${N}.png)
endforeach(S)

set(EXAMPLE_DIRECTORIES
hello_world
server
scaling_doubles
multibindings
simple_injection
)

foreach(D ${EXAMPLE_DIRECTORIES})
  add_custom_command(OUTPUT ${D}-deps.png
                     COMMAND bash < ${CMAKE_CURRENT_SOURCE_DIR}/extract_dependencies.sh > ${CMAKE_CURRENT_BINARY_DIR}/${D}.dot
                     COMMAND dot -Goverlap=prism10000 ${CMAKE_CURRENT_BINARY_DIR}/${D}.dot -Tpng -o ${CMAKE_CURRENT_BINARY_DIR}/${D}-deps.png
                     # This normalizes the PNG files, so that we avoid tracking multiple copies of the same file in the Github wiki repo.
                     COMMAND exiftool -all= -overwrite_original ${CMAKE_CURRENT_BINARY_DIR}/${D}-deps.png
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../examples/${D}
                     DEPENDS
                        ../../examples/${D}
                        extract_dependencies.sh
                     )
  add_custom_target(${D}-deps ALL
                    DEPENDS ${D}-deps.png)
endforeach(D)

