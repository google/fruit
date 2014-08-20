/*
This program binds 200 types, with 100 edges (type->type dependencies) and then injects the 100 X* types.
The whole process is then repeated 1000 times.
It was created with the following bash script:

N=100
M=1000
(
  echo "#include <fruit/fruit.h>"
  for n in `seq 1 $N`
  do
    echo "struct Y$n { INJECT(Y$n()) {} };"
    echo "struct X$n { INJECT(X$n(Y$n)) {} };"
  done
  echo 'fruit::Component<'
  for n in `seq 1 $[$N - 1]`
  do
    echo "X$n,"
  done
  echo "X$N"
  echo "> getComponent() { return fruit::createComponent(); }"
  echo "int main() {"
  echo "for (int i = 0; i < $M; i++) {"
  echo "fruit::Injector<"
  for n in `seq 1 $[$N - 1]`
  do
    echo "X$n,"
  done
  echo "X$N"
  echo "> injector(getComponent());"
  for n in `seq 1 $N`
  do
    echo "injector.get<X$n*>();"
  done
  echo "}"
  echo "return 0;"
  echo "}"
)>main.cpp
*/

#include <fruit/fruit.h>
struct Y1 { INJECT(Y1()) {} };
struct X1 { INJECT(X1(Y1)) {} };
struct Y2 { INJECT(Y2()) {} };
struct X2 { INJECT(X2(Y2)) {} };
struct Y3 { INJECT(Y3()) {} };
struct X3 { INJECT(X3(Y3)) {} };
struct Y4 { INJECT(Y4()) {} };
struct X4 { INJECT(X4(Y4)) {} };
struct Y5 { INJECT(Y5()) {} };
struct X5 { INJECT(X5(Y5)) {} };
struct Y6 { INJECT(Y6()) {} };
struct X6 { INJECT(X6(Y6)) {} };
struct Y7 { INJECT(Y7()) {} };
struct X7 { INJECT(X7(Y7)) {} };
struct Y8 { INJECT(Y8()) {} };
struct X8 { INJECT(X8(Y8)) {} };
struct Y9 { INJECT(Y9()) {} };
struct X9 { INJECT(X9(Y9)) {} };
struct Y10 { INJECT(Y10()) {} };
struct X10 { INJECT(X10(Y10)) {} };
struct Y11 { INJECT(Y11()) {} };
struct X11 { INJECT(X11(Y11)) {} };
struct Y12 { INJECT(Y12()) {} };
struct X12 { INJECT(X12(Y12)) {} };
struct Y13 { INJECT(Y13()) {} };
struct X13 { INJECT(X13(Y13)) {} };
struct Y14 { INJECT(Y14()) {} };
struct X14 { INJECT(X14(Y14)) {} };
struct Y15 { INJECT(Y15()) {} };
struct X15 { INJECT(X15(Y15)) {} };
struct Y16 { INJECT(Y16()) {} };
struct X16 { INJECT(X16(Y16)) {} };
struct Y17 { INJECT(Y17()) {} };
struct X17 { INJECT(X17(Y17)) {} };
struct Y18 { INJECT(Y18()) {} };
struct X18 { INJECT(X18(Y18)) {} };
struct Y19 { INJECT(Y19()) {} };
struct X19 { INJECT(X19(Y19)) {} };
struct Y20 { INJECT(Y20()) {} };
struct X20 { INJECT(X20(Y20)) {} };
struct Y21 { INJECT(Y21()) {} };
struct X21 { INJECT(X21(Y21)) {} };
struct Y22 { INJECT(Y22()) {} };
struct X22 { INJECT(X22(Y22)) {} };
struct Y23 { INJECT(Y23()) {} };
struct X23 { INJECT(X23(Y23)) {} };
struct Y24 { INJECT(Y24()) {} };
struct X24 { INJECT(X24(Y24)) {} };
struct Y25 { INJECT(Y25()) {} };
struct X25 { INJECT(X25(Y25)) {} };
struct Y26 { INJECT(Y26()) {} };
struct X26 { INJECT(X26(Y26)) {} };
struct Y27 { INJECT(Y27()) {} };
struct X27 { INJECT(X27(Y27)) {} };
struct Y28 { INJECT(Y28()) {} };
struct X28 { INJECT(X28(Y28)) {} };
struct Y29 { INJECT(Y29()) {} };
struct X29 { INJECT(X29(Y29)) {} };
struct Y30 { INJECT(Y30()) {} };
struct X30 { INJECT(X30(Y30)) {} };
struct Y31 { INJECT(Y31()) {} };
struct X31 { INJECT(X31(Y31)) {} };
struct Y32 { INJECT(Y32()) {} };
struct X32 { INJECT(X32(Y32)) {} };
struct Y33 { INJECT(Y33()) {} };
struct X33 { INJECT(X33(Y33)) {} };
struct Y34 { INJECT(Y34()) {} };
struct X34 { INJECT(X34(Y34)) {} };
struct Y35 { INJECT(Y35()) {} };
struct X35 { INJECT(X35(Y35)) {} };
struct Y36 { INJECT(Y36()) {} };
struct X36 { INJECT(X36(Y36)) {} };
struct Y37 { INJECT(Y37()) {} };
struct X37 { INJECT(X37(Y37)) {} };
struct Y38 { INJECT(Y38()) {} };
struct X38 { INJECT(X38(Y38)) {} };
struct Y39 { INJECT(Y39()) {} };
struct X39 { INJECT(X39(Y39)) {} };
struct Y40 { INJECT(Y40()) {} };
struct X40 { INJECT(X40(Y40)) {} };
struct Y41 { INJECT(Y41()) {} };
struct X41 { INJECT(X41(Y41)) {} };
struct Y42 { INJECT(Y42()) {} };
struct X42 { INJECT(X42(Y42)) {} };
struct Y43 { INJECT(Y43()) {} };
struct X43 { INJECT(X43(Y43)) {} };
struct Y44 { INJECT(Y44()) {} };
struct X44 { INJECT(X44(Y44)) {} };
struct Y45 { INJECT(Y45()) {} };
struct X45 { INJECT(X45(Y45)) {} };
struct Y46 { INJECT(Y46()) {} };
struct X46 { INJECT(X46(Y46)) {} };
struct Y47 { INJECT(Y47()) {} };
struct X47 { INJECT(X47(Y47)) {} };
struct Y48 { INJECT(Y48()) {} };
struct X48 { INJECT(X48(Y48)) {} };
struct Y49 { INJECT(Y49()) {} };
struct X49 { INJECT(X49(Y49)) {} };
struct Y50 { INJECT(Y50()) {} };
struct X50 { INJECT(X50(Y50)) {} };
struct Y51 { INJECT(Y51()) {} };
struct X51 { INJECT(X51(Y51)) {} };
struct Y52 { INJECT(Y52()) {} };
struct X52 { INJECT(X52(Y52)) {} };
struct Y53 { INJECT(Y53()) {} };
struct X53 { INJECT(X53(Y53)) {} };
struct Y54 { INJECT(Y54()) {} };
struct X54 { INJECT(X54(Y54)) {} };
struct Y55 { INJECT(Y55()) {} };
struct X55 { INJECT(X55(Y55)) {} };
struct Y56 { INJECT(Y56()) {} };
struct X56 { INJECT(X56(Y56)) {} };
struct Y57 { INJECT(Y57()) {} };
struct X57 { INJECT(X57(Y57)) {} };
struct Y58 { INJECT(Y58()) {} };
struct X58 { INJECT(X58(Y58)) {} };
struct Y59 { INJECT(Y59()) {} };
struct X59 { INJECT(X59(Y59)) {} };
struct Y60 { INJECT(Y60()) {} };
struct X60 { INJECT(X60(Y60)) {} };
struct Y61 { INJECT(Y61()) {} };
struct X61 { INJECT(X61(Y61)) {} };
struct Y62 { INJECT(Y62()) {} };
struct X62 { INJECT(X62(Y62)) {} };
struct Y63 { INJECT(Y63()) {} };
struct X63 { INJECT(X63(Y63)) {} };
struct Y64 { INJECT(Y64()) {} };
struct X64 { INJECT(X64(Y64)) {} };
struct Y65 { INJECT(Y65()) {} };
struct X65 { INJECT(X65(Y65)) {} };
struct Y66 { INJECT(Y66()) {} };
struct X66 { INJECT(X66(Y66)) {} };
struct Y67 { INJECT(Y67()) {} };
struct X67 { INJECT(X67(Y67)) {} };
struct Y68 { INJECT(Y68()) {} };
struct X68 { INJECT(X68(Y68)) {} };
struct Y69 { INJECT(Y69()) {} };
struct X69 { INJECT(X69(Y69)) {} };
struct Y70 { INJECT(Y70()) {} };
struct X70 { INJECT(X70(Y70)) {} };
struct Y71 { INJECT(Y71()) {} };
struct X71 { INJECT(X71(Y71)) {} };
struct Y72 { INJECT(Y72()) {} };
struct X72 { INJECT(X72(Y72)) {} };
struct Y73 { INJECT(Y73()) {} };
struct X73 { INJECT(X73(Y73)) {} };
struct Y74 { INJECT(Y74()) {} };
struct X74 { INJECT(X74(Y74)) {} };
struct Y75 { INJECT(Y75()) {} };
struct X75 { INJECT(X75(Y75)) {} };
struct Y76 { INJECT(Y76()) {} };
struct X76 { INJECT(X76(Y76)) {} };
struct Y77 { INJECT(Y77()) {} };
struct X77 { INJECT(X77(Y77)) {} };
struct Y78 { INJECT(Y78()) {} };
struct X78 { INJECT(X78(Y78)) {} };
struct Y79 { INJECT(Y79()) {} };
struct X79 { INJECT(X79(Y79)) {} };
struct Y80 { INJECT(Y80()) {} };
struct X80 { INJECT(X80(Y80)) {} };
struct Y81 { INJECT(Y81()) {} };
struct X81 { INJECT(X81(Y81)) {} };
struct Y82 { INJECT(Y82()) {} };
struct X82 { INJECT(X82(Y82)) {} };
struct Y83 { INJECT(Y83()) {} };
struct X83 { INJECT(X83(Y83)) {} };
struct Y84 { INJECT(Y84()) {} };
struct X84 { INJECT(X84(Y84)) {} };
struct Y85 { INJECT(Y85()) {} };
struct X85 { INJECT(X85(Y85)) {} };
struct Y86 { INJECT(Y86()) {} };
struct X86 { INJECT(X86(Y86)) {} };
struct Y87 { INJECT(Y87()) {} };
struct X87 { INJECT(X87(Y87)) {} };
struct Y88 { INJECT(Y88()) {} };
struct X88 { INJECT(X88(Y88)) {} };
struct Y89 { INJECT(Y89()) {} };
struct X89 { INJECT(X89(Y89)) {} };
struct Y90 { INJECT(Y90()) {} };
struct X90 { INJECT(X90(Y90)) {} };
struct Y91 { INJECT(Y91()) {} };
struct X91 { INJECT(X91(Y91)) {} };
struct Y92 { INJECT(Y92()) {} };
struct X92 { INJECT(X92(Y92)) {} };
struct Y93 { INJECT(Y93()) {} };
struct X93 { INJECT(X93(Y93)) {} };
struct Y94 { INJECT(Y94()) {} };
struct X94 { INJECT(X94(Y94)) {} };
struct Y95 { INJECT(Y95()) {} };
struct X95 { INJECT(X95(Y95)) {} };
struct Y96 { INJECT(Y96()) {} };
struct X96 { INJECT(X96(Y96)) {} };
struct Y97 { INJECT(Y97()) {} };
struct X97 { INJECT(X97(Y97)) {} };
struct Y98 { INJECT(Y98()) {} };
struct X98 { INJECT(X98(Y98)) {} };
struct Y99 { INJECT(Y99()) {} };
struct X99 { INJECT(X99(Y99)) {} };
struct Y100 { INJECT(Y100()) {} };
struct X100 { INJECT(X100(Y100)) {} };
fruit::Component<
X1,
X2,
X3,
X4,
X5,
X6,
X7,
X8,
X9,
X10,
X11,
X12,
X13,
X14,
X15,
X16,
X17,
X18,
X19,
X20,
X21,
X22,
X23,
X24,
X25,
X26,
X27,
X28,
X29,
X30,
X31,
X32,
X33,
X34,
X35,
X36,
X37,
X38,
X39,
X40,
X41,
X42,
X43,
X44,
X45,
X46,
X47,
X48,
X49,
X50,
X51,
X52,
X53,
X54,
X55,
X56,
X57,
X58,
X59,
X60,
X61,
X62,
X63,
X64,
X65,
X66,
X67,
X68,
X69,
X70,
X71,
X72,
X73,
X74,
X75,
X76,
X77,
X78,
X79,
X80,
X81,
X82,
X83,
X84,
X85,
X86,
X87,
X88,
X89,
X90,
X91,
X92,
X93,
X94,
X95,
X96,
X97,
X98,
X99,
X100
> getComponent() { return fruit::createComponent(); }
int main() {
for (int i = 0; i < 1000; i++) {
fruit::Injector<
X1,
X2,
X3,
X4,
X5,
X6,
X7,
X8,
X9,
X10,
X11,
X12,
X13,
X14,
X15,
X16,
X17,
X18,
X19,
X20,
X21,
X22,
X23,
X24,
X25,
X26,
X27,
X28,
X29,
X30,
X31,
X32,
X33,
X34,
X35,
X36,
X37,
X38,
X39,
X40,
X41,
X42,
X43,
X44,
X45,
X46,
X47,
X48,
X49,
X50,
X51,
X52,
X53,
X54,
X55,
X56,
X57,
X58,
X59,
X60,
X61,
X62,
X63,
X64,
X65,
X66,
X67,
X68,
X69,
X70,
X71,
X72,
X73,
X74,
X75,
X76,
X77,
X78,
X79,
X80,
X81,
X82,
X83,
X84,
X85,
X86,
X87,
X88,
X89,
X90,
X91,
X92,
X93,
X94,
X95,
X96,
X97,
X98,
X99,
X100
> injector(getComponent());
injector.get<X1*>();
injector.get<X2*>();
injector.get<X3*>();
injector.get<X4*>();
injector.get<X5*>();
injector.get<X6*>();
injector.get<X7*>();
injector.get<X8*>();
injector.get<X9*>();
injector.get<X10*>();
injector.get<X11*>();
injector.get<X12*>();
injector.get<X13*>();
injector.get<X14*>();
injector.get<X15*>();
injector.get<X16*>();
injector.get<X17*>();
injector.get<X18*>();
injector.get<X19*>();
injector.get<X20*>();
injector.get<X21*>();
injector.get<X22*>();
injector.get<X23*>();
injector.get<X24*>();
injector.get<X25*>();
injector.get<X26*>();
injector.get<X27*>();
injector.get<X28*>();
injector.get<X29*>();
injector.get<X30*>();
injector.get<X31*>();
injector.get<X32*>();
injector.get<X33*>();
injector.get<X34*>();
injector.get<X35*>();
injector.get<X36*>();
injector.get<X37*>();
injector.get<X38*>();
injector.get<X39*>();
injector.get<X40*>();
injector.get<X41*>();
injector.get<X42*>();
injector.get<X43*>();
injector.get<X44*>();
injector.get<X45*>();
injector.get<X46*>();
injector.get<X47*>();
injector.get<X48*>();
injector.get<X49*>();
injector.get<X50*>();
injector.get<X51*>();
injector.get<X52*>();
injector.get<X53*>();
injector.get<X54*>();
injector.get<X55*>();
injector.get<X56*>();
injector.get<X57*>();
injector.get<X58*>();
injector.get<X59*>();
injector.get<X60*>();
injector.get<X61*>();
injector.get<X62*>();
injector.get<X63*>();
injector.get<X64*>();
injector.get<X65*>();
injector.get<X66*>();
injector.get<X67*>();
injector.get<X68*>();
injector.get<X69*>();
injector.get<X70*>();
injector.get<X71*>();
injector.get<X72*>();
injector.get<X73*>();
injector.get<X74*>();
injector.get<X75*>();
injector.get<X76*>();
injector.get<X77*>();
injector.get<X78*>();
injector.get<X79*>();
injector.get<X80*>();
injector.get<X81*>();
injector.get<X82*>();
injector.get<X83*>();
injector.get<X84*>();
injector.get<X85*>();
injector.get<X86*>();
injector.get<X87*>();
injector.get<X88*>();
injector.get<X89*>();
injector.get<X90*>();
injector.get<X91*>();
injector.get<X92*>();
injector.get<X93*>();
injector.get<X94*>();
injector.get<X95*>();
injector.get<X96*>();
injector.get<X97*>();
injector.get<X98*>();
injector.get<X99*>();
injector.get<X100*>();
}
return 0;
}
