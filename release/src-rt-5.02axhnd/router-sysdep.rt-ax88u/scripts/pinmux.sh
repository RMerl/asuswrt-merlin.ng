# read/write pinmux configuration
case $1 in
-help) cat <<EOF
Usage:
  $0 <pin> <pin> ...
     displays pinmux configuration for each pin
  $0 -mux <mux> <pin> <pin> ...
     configures each pin with mux value
EOF
exit ;;
-mux) mux=$2; shift 2 ;;
esac
hex=0123456789abcdef
for pin; do
  # 15:12 write data
  # 11:00 address
  lsb=${hex:$((pin/16)):1}${hex:$((pin%16)):1}
  # 5:5 1=pm enable
  # 4:4 0=unreset
  # 3:0 {1,3}={write,read} pinmux values
  case $mux in
  '') sw $tpdatamsb 0 $lsb 23 && set $(dw $tpdiagrdl)
      let s=0x$3; o="$pin: $3"
      for f in final_select:3 :1 select:3 :1 select_prevent:3 :1 \
        pre_select:3 :1 sel_out_final:4 sel_out:4 pull_out:3 i_din_PAD:1 \
        o_dout_PAD:1 o_oeb_PAD:1 o_pdn_PAD:1 o_pup_PAD:1; do
          let m="1<<${f#*:}"; let v="$s&$m-1" s=$s/$m
          test -n "${f%:*}" && o="$o ${f%:*}=$v"
      done
      echo $o ;;
  *)  sw $tpdatamsb 0 ${mux}0$lsb 21 ;;
  esac
done
