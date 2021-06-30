/*
 * HEADER Testing run_exec_command()
 */

struct test_data {
    const char *command;
    const char *input;
    const char *expected_output;
    int res;
};

static struct test_data test_data[] = {
    { "/bin/sh -c 'echo a'", NULL, "a\n", 0 },
    { "/bin/sh -c 'echo a  b'", NULL, "a b\n", 0 },
    { "/bin/sh -c 'echo $*' . 0  1  2  3  4  5  6  7  8  9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41  42  43  44  45  46  47  48  49  50  51  52  53  54  55  56  57  58  59  60  61  62  63  64  65  66  67  68  69  70  71  72  73  74  75  76  77  78  79  80  81  82  83  84  85  86  87  88  89  90  91  92  93  94  95  96  97  98  99",
      NULL,
      "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99\n",
      0 },
    { "/bin/cat", "b  c\n", "b  c\n", 0 },
    { "/bin/sh -c 'false'", NULL, NULL, 1 },
    { "/bin/sh -c 'exit 77'", NULL, NULL, 77 },
};

#if 0
snmp_set_do_debugging(TRUE);
debug_register_tokens("run:exec");
#endif

{
    char output[512];
    int output_len;
    int i;

    for (i = 0; i < sizeof(test_data) / sizeof(test_data[0]); i++) {
        const struct test_data *p = &test_data[i];
        int res;

        output_len = sizeof(output);
        res = run_exec_command(p->command, p->input, output, &output_len);
        OKF(res == p->res, ("%s: return value %d <> %d", p->command, res,
                            p->res));
        if (res == p->res && p->expected_output) {
            OKF(strcmp(output, p->expected_output) == 0,
                ("%s: output %s <> %s", p->command, output,
                 p->expected_output));
        }
    }
}
