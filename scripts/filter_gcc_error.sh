#!/bin/bash

export CurPos=0
export TOKENS=()

mergeInSingleLine() {
  OPENING_TOKEN="${TOKENS[$CurPos]}"
  echo -n "${OPENING_TOKEN}"
  CurPos=$[$CurPos + 1]
  if ! [[ "${OPENING_TOKEN}" =~ \< ]] && ! [[ "${OPENING_TOKEN}" =~ \( ]]
  then
    # This is not an opening token, it's an atomic component.
    return
  fi
  while true
  do
    if ! [[ "${TOKENS[$CurPos]}" =~ \> ]] && ! [[ "${TOKENS[$CurPos]}" =~ \) ]]
    then
      mergeInSingleLine
    else
      if [[ "${TOKENS[$CurPos]}" =~ \< ]] || [[ "${TOKENS[$CurPos]}" =~ \( ]]
      then
        # Both an opening and a closing token, print it and go ahead.
        echo -n "${TOKENS[$CurPos]}"
        CurPos=$[$CurPos + 1]
      else
        break
      fi
    fi
  done
  echo -n "${TOKENS[CurPos]}"
  CurPos=$[$CurPos + 1]
}

considerMerging() {
  OPENING_TOKEN="${TOKENS[CurPos]}"
  if ! [[ "${OPENING_TOKEN}" =~ \< ]] && ! [[ "${OPENING_TOKEN}" =~ \( ]]
  then
    # This is not an opening token, it's an atomic component.
    echo "${OPENING_TOKEN}"
    CurPos=$[$CurPos + 1]
    return;
  fi
  if [[ "${OPENING_TOKEN}" = "Type<" ]]
  then
    # Type<...> should be on a single line.
    mergeInSingleLine
    echo
    return;
  fi
  echo "${OPENING_TOKEN}"
  CurPos=$[$CurPos + 1]
  while true
  do
    if ! [[ "${TOKENS[$CurPos]}" =~ \> ]] && ! [[ "${TOKENS[$CurPos]}" =~ \) ]]
    then
      considerMerging
    else
      if [[ "${TOKENS[$CurPos]}" =~ \< ]] || [[ "${TOKENS[$CurPos]}" =~ \( ]]
      then
        # Both an opening and a closing token, print it and go ahead.
        echo "${TOKENS[$CurPos]}"
        CurPos=$[$CurPos + 1]
      else
        break
      fi
    fi
  done
  echo "${TOKENS[$CurPos]}"
  CurPos=$[$CurPos + 1]
}

while read line
do
  if [[ "${line}" =~ required\ from\ .*DoEval\< ]]
  then
    echo
    IFS=$'\r\n' GLOBIGNORE='*' :;
    TOKENS=($(echo "${line}" |
        sed 's| (\*)||g;
              s|.* required from .*DoEval<||;
              s|>.$||;
              s/fruit::impl::meta:://g;
              s| >|>|g;
              s|, |,|g' |
        sed 's|[>]|,>|g;
              s|[)]|,)|g' |
        sed 's|[<]|<\n|g;
              s|[(]|(\n|g;
              s|[,]|,\n|g' |
        grep -v "^,$" ))
    considerMerging |
    sed 's|,>|>|g;
         s|,)|)|g' |
    awk -F@ '/^[^()<>]*[>)][^()<>]*[(<][^()<>]*$/ {curIndent-=2; for (i=0; i<curIndent; i++) { printf(" "); } print; curIndent+=2; next;}
             /[()<>].*[()<>]/ || !/[()<>]/ {for (i=0; i<curIndent; i++) { printf(" "); } print; next;}
             /[(<]/ { for (i=0; i<curIndent; i++) { printf(" "); } print; curIndent+=2; next; }
             /[)>]/ || /^[^()<>]*>/ { curIndent-=2; for (i=0; i<curIndent; i++) { printf(" "); } print; next; }
             '
  elif [[ "${line}" =~ required\ from\ .*EvalFun\< ]]
  then
    echo
    IFS=$'\r\n' GLOBIGNORE='*' :;
    TOKENS=($(echo "${line}" |
        sed 's|,|(|' |
        sed 's| (\*)||g;
              s|.* required from .*EvalFun<||;
              s|>.$||;
              s/fruit::impl::meta:://g;
              s| >|>|g;
              s|, |,|g' |
        sed 's|[>]|,>|g;
              s|[)]|,)|g' |
        sed 's|[<]|<\n|g;
              s|[(]|(\n|g;
              s|[,]|,\n|g' |
        grep -v "^,$";
        echo ')'))
    considerMerging |
    sed 's|,>|>|g;
         s|,)|)|g' |
    awk -F@ '/^[^()<>]*[>)][^()<>]*[(<][^()<>]*$/ {curIndent-=2; for (i=0; i<curIndent; i++) { printf(" "); } print; curIndent+=2; next;}
             /[()<>].*[()<>]/ || !/[()<>]/ {for (i=0; i<curIndent; i++) { printf(" "); } print; next;}
             /[(<]/ { for (i=0; i<curIndent; i++) { printf(" "); } print; curIndent+=2; next; }
             /[)>]/ || /^[^()<>]*>/ { curIndent-=2; for (i=0; i<curIndent; i++) { printf(" "); } print; next; }
             '
  else
    echo "${line}"
  fi
done | sed 's/fruit::impl::meta:://g;s/struct //g' | grep -v "required from .EvalIf<"
