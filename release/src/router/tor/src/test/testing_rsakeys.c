/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "lib/crypt_ops/crypto_rand.h"
#include "orconfig.h"
#include "core/or/or.h"
#include "test/test.h"

/** Define this if unit tests spend too much time generating public keys.
 * This module is meant to save time by using a bunch of pregenerated RSA
keys among */
#define USE_PREGENERATED_RSA_KEYS

#ifdef USE_PREGENERATED_RSA_KEYS

static const char *PREGEN_KEYS_1024[] = {
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICWwIBAAKBgQCZa39BCgq7KWBWFSjGYHhqmTCHvQ7WNEFAb9Mujb6Xn/Zy01fu\n"
"WIpVvqmAKeLNEziItUm/gB8GwAN+/ZLwL9pufjIp2Ar+yqVXKySioZQxuCgTP2wm\n"
"Ku0OfmAra1Xbtrkc2OCJllxkyNPrJ/kxfwjWR96UP0+VMbOlkBoEH1FtvwIDAQAB\n"
"AoGAUXoygeMIYe+OdwkTt48CRHKIwH3aRE5KHSOGPyIOB05vvvmYqD8jcHgqYqNc\n"
"DNdZXdkRin9LevU8phObFq4DTXp08XggUx4Kk4AdsFKubQtJ8gHm3xlSKbZXX2m/\n"
"ZF0GRaZtVDQ3TRGh+OBLILt/2jT+BaFKGAyJ7al76F2nprECQQDJyLlteLDFBmrd\n"
"0kAjNBE50S5YskBCQeQACROfyTKW8lG1J57UBeYjXvbrDFBR4alIS9DEexGai9Gz\n"
"wxpgKg2nAkEAwqQmPstjHxvqGQRi41uXO026MLxY7dhEqs1aSw3tuT8v17pW3OEa\n"
"Qxv7JINePZ3+sNN+Ic+3RXBR0QuD7lSSKQJAZjVSF21GvMXfY7SX4D0DbLHUNAE2\n"
"I1mUz5/JXOpgwazETmpfPS4vwELd93kpRhBz2rbsbFmaNRoVgmSU+5jRiQJAZ1bV\n"
"g2NilgKxEGU2x3U6Xt8Oqo9lO6omEvUCKnUTsNWuZf/l3FGbKuQxO5qPr3Ex5tny\n"
"zqrEqBZRKgbOHfxCuQJAbJY5C3Nm5koemr031r00MY2YD1b6+hyKZyPdZ21HpyY8\n"
"z1kWShL0POjYPX/BnKE1FkpklWcKBb7wkK7dvAKkEQ==\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXQIBAAKBgQCyqMM2TfFGV5tVBTVabxLVln8146nDavIdR6q78DCUMh8Zfzkk\n"
"h9Lbl1NX4RU+AmrCZMPq21/EjIRxRQyRdgPYJVLdp96eGeYnEzmMkqvXiswXvDg/\n"
"tXqsjyJeYsoHMQWDTpCLfjYo4K1ol1sg8VIs4wQeq5og6QSdmhBoz7MyqQIDAQAB\n"
"AoGBAIJekey7nZeV8Bxva4ptSRIg+v0I/2VBUiG5nUX9NIW/uV/yrXERx/VDjKaw\n"
"8b5JJzxpKWnk4RJc83xwRYaT1qMYHiQfybxEI0K9SjhtaThAjtXkQGtZgLJILl3t\n"
"yh3LPTh1ocwafsKjU6eGYAe/DYn9/QwYHbtyaimcigu4etp9AkEA2DgC+HndoP1i\n"
"np26Lx+4TG0vAfrVYGSLT9FXwf2iBV3oJvdKqu6wr8ipb1SbshRPcOQd31/mCh6+\n"
"2BR+d4ddcwJBANOHrlBbGZdHnoEu6kKbPwwkc31IZYqyfSpkqm0Lb2oWZ9SInKfc\n"
"cz0qpH91p610XUpYmycaJr4K+N8jgrz86HMCQQCoqGBg1Ca2OpCf66bctWB8dTqS\n"
"z8d7rlIhC8npr1+f0hWRt5pN5Wx7YgoQpq3gZgllpPtMT7DQOhVh1fKkaDnTAkA4\n"
"XuskPPLX7t0dvhvtviOSH9CrLXTp/mD+wC7uumJpmij3aaSd01DelxOZaAhUYDNQ\n"
"UcafKAf1E0V5aaQ4qwljAkA9NVN6CtpzzcLrstTKxrx5P1Ylt/0UYQDo1lIaqwrT\n"
"aOFbXmOungiC9+p/4U7RbX0MEzjFDHCWlaHASviGVgta\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICWwIBAAKBgQDDt2V63APj3JSqaRgofUzhtB+prm0wII4uHyxfOxnpYIELOW5z\n"
"3UHmkr+B4D+Nif5jIp0i6W4OS4S+YHewKsDsXvXKRIW78KzOt6Le4JI9rSarNjy5\n"
"aJKksWQRALLCmxP/BdolaBFqF3fIPD5+Zxu8ESgxhkEQI4p7awUp3E730QIDAQAB\n"
"AoGAZktfAR4p8lkCYydW9yK2ommQ+xEuBK+fYL/uYz/yxSYpjIJSFsEYhrlA21Mo\n"
"JIRxr8MRuoOjgFk8YnztUeimuHpslDlZDaCBzjRjBRFCMepZNG9xqSEL0u7C+SH6\n"
"KU5f2x2P6PneBj6WaHZM+6Lf2xHlOoeuaVSUfq2Pk2VBF9kCQQDtawWWNwP0+xea\n"
"oCAQpanaLzYPjlqZfHJQ1AAI5eSkdf1qmlypIHwOtjAEa6XuEO/Or8RNkNy4nQdw\n"
"qhcQ7PXDAkEA0wjT6Z+Lrt67FnwPgoSvl4Nukcqw4OWHbBKhaQPsO9+oc3PAXLdD\n"
"SclUUqDF6NX1yONTV1KrPdz4zElmEua+2wJABm4inZnp2oW+cuqpU6oY+pbSwQMb\n"
"AxMyyWukgJkxYx7q+SsrHU2K7p8Sl9wOh28f/5oVGAC3aayfGfcRXtz8HwJAIqeO\n"
"dQzYGU1GF7kjquEzHIRewd4xEZ1fkaW1j9MvFd3ygZL+gbsud41yJWd1WHjaNbTu\n"
"2KYgrLX+vT1IX844hQJAbg0V7iHlttQqXL7yN09jIjQLprqVhDZCUHS9s9Dxe7fz\n"
"Ac0ZZD0D6EVNmSmBB71q7kLUWX/W/10d447TLnnfew==\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXgIBAAKBgQDhCAjPEockl4lqkvoIb5O3NJJG8NWD31c63e/cPWY6MX5nOM/q\n"
"avof2eWJxFOk0HQ2BRVwIgNex6kLxtsdw7XE0A5uZorTp9DbRCGMqUqHNhHH9ci2\n"
"mMPP9jptq3ieWg310bH4Tad8h3WE2npSCDBvxyV6EmuH2rlQW9ZlHNoiRQIDAQAB\n"
"AoGBAI4PgWggPTqng7PJF5mNvsYQpSutzE0VCL977nmuNUQVjMPjRLarVD4ZU+QW\n"
"EevhQQv9R5xjjJcgGqL5pchzjeKDm0/LA+AygnZoDMs2O68Neieqvr7cPqr5ALGs\n"
"WuZvSn+bRJTenvV9sUh2ii0/u3GQbL1v7GWDkIdD7itDbmRhAkEA8iijuEY+W67w\n"
"7JusjY2MQ2Cm6xxxR0YcnYPzT6UDm+Z7NNJwKscQ6AjayNmxmXGpbUdukzLzXf8y\n"
"fccI9t6iHQJBAO3kx9nZay0Ktl51QP5o2gwoqRIbnogGfR06KJOlzIPGR0aPn8cg\n"
"uKq2SiyjewEaSBM6S/4UlxYUmvc3VKnxCEkCQQDpTjg2YQ7RPGIIRA/iLV7Wx3bq\n"
"C/QjjCwjoi44LK6mdE9928WPoUzrkSRg4EQYpwZqL6kcDrmkdSuLPMipOGQNAkA3\n"
"KtzlujPOiDNuiEaAORSHyU4b8ue6p7aP9pK+Wq6oyGxzAo+NABuTCx78ZxT5Vnzs\n"
"aJKC44d+CV0+g0hQ+KJxAkEAqFYzNWIzTHX8DVDdK9BpUaBg1DFxIeP5Kk+/X3FF\n"
"5BafG08B6OiLf8qIGGsxLXNRjIE0GVp3Sy23FUKtUymP+A==\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXQIBAAKBgQDMDk01VwPxQq/BAwOBmfGUP/x5BQn+uxI0Aat6bdWuz/2CsjbS\n"
"CWD/YLCaPm+DpHp9RMwk4HONJaw4B2XOw3ELPx7y9DEgdC1wZ9wRkJmqr2IJZoZR\n"
"C7x43nNv+/IXTiRkkljCcMpoL1Tld+L2VbmWR29PdZwvspWRILkEZu1mNwIDAQAB\n"
"AoGANvFK3KfXSei4xfF3yjeXEmHAKx2uOUZJenNQpqBYPr+F9ODjXd5knZ59LqrM\n"
"/9cTnBMgHHXK5yBTpKppQSjikLeQ2BF04Ktff9oGqVcS9x/rKo0CREuxsEfawZOW\n"
"OzOWENp4YcDKGP1I/Ctr185QzStaWrXVQftxmYQ53T77ShECQQDnhabwtqW7rfe4\n"
"+MfkWEJ9Y2s6iMs3JWnwPOX9G9R39PiAD4vAghHJyHHttS9Ipxmvp0hThu0x7a4g\n"
"8BfUpqgjAkEA4aFAmzarWKigREAACVTYH2RHpXbuk05vF9WqfMPiEvQUd5a1q6vc\n"
"xkGZsE3v/TExLjPRZP4FeUNV5sD7THzA3QJBAJxPoRlNx3GCEAlDdfnWGPX9JI09\n"
"hC40RWUcSI7ttjJTI1+an1kWuBnLChhaRpU/tFjikTNLmmMmPHUihIRfDI8CQG7g\n"
"3WzpKr8A7vFbOilbxnF2yDaqAYfmTXW7DHMPl/OUetJh/5kDdhT/e9VGF5+nIvH/\n"
"iPFGW85Bpt8lCtmFnQkCQQDjpp9iy2qesE7KKX4Kv3++QfCJ2w3g7lwg4iyncoDd\n"
"JrM53p29HROM21R6eekvqeWIe9tEX754b+E/N60ZjpGm\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXgIBAAKBgQDdDn3H+Eu0AW5GKohqDBntw6ubnd3VaJwZGzZyga4J2kLg8peP\n"
"RAW6GDD6pcHzW+KZbFWHtRk70FSwvmyGcf+DY0r5tfyCHyDGmbJyPR0o6OVCgSFl\n"
"ccf4eDvbyszzMdlx3uL05ABIpCShoKtEUqvyIQla3Jon+QBwuVkizMzyVwIDAQAB\n"
"AoGACoKh4Fwh3VEkGRn0mnYw1Wk0Q5Xh8j+jDF6K3C7mQ3mpLGDca+dkDlEQIxq2\n"
"egeoYnsQJf+qT3m8TRsAtfO9nj7+7IX4BfCtdIi4RNcorbs5YMWtFyaywnM6SQjS\n"
"+1qf74aL4On9WRO2FtvnTMjFAAkiWNbQp7mWwTmB59i620ECQQDwde6/PwhUzvZh\n"
"dyslKJdna5RjkDQyDIuh0zD/tFZ0Iko7Luec8q6n52ev/n0OiTLGetUh8goePsPP\n"
"HVZHidNJAkEA61eMCmmu+GCAg2vJRtL5sDakAXsbP5M9Bf/QVHXtc4EVXHC6T2ld\n"
"bldOJriNbBThBuPNmlQbssn9FApkyWT4nwJBAIuHIv3+CUuMvBJaH8L0BsaP+g67\n"
"wk24Ud2Yujnl3rSMoR4uXV8IwqfS8quAs/gXTEs3QyzrUUuzh9NKZqIkK2ECQQCz\n"
"vivBEDKIlPvSZBJYO25kfXcJgoKvLb9fw5/TwjXXD/HGpnpFiI3JZnjT7gRlVhT/\n"
"9CDmC/MTvF3EXqPXhXy1AkEAo3a2me23Ljmub21jycSKaCk09dK85QTRRMe9c/hs\n"
"i+pcGi9ZZW0Mm7cyQo47oXjNurkkv0fEvXIobVTEXAGU7w==\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXAIBAAKBgQCv8R1IbfYnE3R3kNeezJ7m02XnyCBDDy0YfrQldQ+urdg1CFye\n"
"bO0iPniJb8fmV8NW7x6nUZTDznCg+igroKXtK/w0WYmJJiH4A7Oi5xNjAfRIPvJ/\n"
"J5GI8szS8rH8tp8pW1h8k/kNg2pnBjwQ2U9omhp95RGaHDQSRYzzH/fEFQIDAQAB\n"
"AoGAcy7+BcH/iZuB/xjzIIJDcUhqibCJ9n0D/+pLU85sYuZrCmUcBZe4M1gEn61v\n"
"iExilRJc1hthskL/l1POYql8lk+aqeeDuh38fWJj60TCV/sENiuXOsTmoFVA5pNn\n"
"lwlG8JlpBMsgr1fGqg1C/WLFfMmvXdKVGvpRqI06j7AYUa0CQQDfZ5rI+FhXBlxo\n"
"PR5CM1LB90DuHUMW+Kqoj0c9d2esXEQM7UqQ/9BiBQbL6Py7Z3VwCxibOqyz7+V7\n"
"2aGUMAKnAkEAyZy5Mu2tHs6YBBxPYam7huzMUYjddN7ixAZUyGwxQp9kTIF2NbSQ\n"
"yVDjKrco3s2lO4qj4pSumwVe3GGlsi6G4wJAOOS3pIqqZK84BUvbUtyjLMZ9AKbv\n"
"GQCG5ZpneB3ahyiQJAKiRL8BIJVLH87b3hYA8GHDCHUu2jwz4xCPd5+qbQJAV0TP\n"
"pYvb9AnZI25drhiaY7z8dA6aTYxs/A0Bhf/PEteLwtIHKRgP1BR/QG4n8slxTGSm\n"
"q91P9ypL9XkPECGzoQJBAIMvGEM7ZGevQHBjJ8HhU8IsgT4cYH/XEYb8jRy4F+Ui\n"
"jKxHPxLuFK4urAZunNUNrqhT0PxbB7hRjtHZrmFkrcc=\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXQIBAAKBgQDSpmV8ncLwc8gXzdFsZGPDtMO7C/IN9jKCIK13WIseMg1APlMt\n"
"PB5lMQ9fa3m9ZRU0L8HzRo+u/Xdos3yIBI38X2Avy0laGKnQxiOKaDT/5ZHeiBBh\n"
"nMZjP2WY5V1sgqNP9RD8enE6WaSvq1j0BM++mn9KEe//5+dWD8tboBKF4QIDAQAB\n"
"AoGBALgVoerdE1Z+WAY1XyaSNHz6o3H6ZnW9CTaex/jb7/dbVikmThnhx842qXCB\n"
"w8m3ZGhOs/edWkNaTde5wsI6+LhVGco/PWxN4v61jokxUU+5KvUvGacXhXIjzKwG\n"
"DrNCYmle62QCI1z4+TLQW/Lq+jw2Wzk70NWEvoP58gt5SJoBAkEA9wubRKRs49LW\n"
"5JNQZ9hjc+mAfP9YK/sMe4jkdloMMWXjSMlF3Z4mI9XQSpfbBqwWIBXsjU/15LIS\n"
"ftmujZsMKQJBANpJEZI7UFoRdSP7AlM0YJuXWnVGyn/K+VIeEso5AlZdKXCTpxqp\n"
"9blWq0UVC6jLesZ5UNPuBiAnrBaVwDA8YvkCQF+FQVfdK607TJO80g4VAP9EfcXX\n"
"BUScIUtytsN8NdKzzpnKGRWDnMOmXI87ABkoWLW3RGuvSyhOIhCiInfmR2ECQASc\n"
"FmroJcJBLCAeZOYs7P1cLOTdIdmhB7LcP7lVit8YCJAADj9Z536KfgNvdleSNH2M\n"
"glB3blmvfMrdTrm2DMECQQDj6GJ/Tc2rCsq534xknasVjrgtJMQFxmQCTVgBx9pc\n"
"gTflJAHAmNDvstacVqeObLCF2ZIvya8fSXGbDOJYeGDv\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXAIBAAKBgQDGgUJAm7vf/3focNGwzv4TkzYF2XwpAirnb61dyxvfug1zKv2k\n"
"AUg3qACiurR7JrI+kAbmxEnNaKV7ts7uO763wP9KE8YAuFZsp7NFA295rEZhw38T\n"
"rUlWHMCeaZ3mqW2q8gA14C/ZJCG4gS91SIHLjNGsbHwr2Jvri2ItwIP8FQIDAQAB\n"
"AoGAONceb32oiHWQkkBr6uL6ogRPPdGO2fdC7c5uqCLWsnOGEmpHAsVTNoym0fIA\n"
"aBsmgv+e2klukKDccdZg3prA+z7lHcc2a4bIFguF6ei80hLIis/dds66fFXofCzy\n"
"DMlkncSbJwIvQHG9gblxp9qSKElZF7XjABZEImarfUlakGkCQQD//msGy5N0ZhMI\n"
"yGMXkwXRJXfmRrIrOqHx6u1eUp4OuqDW+hBz4KCHnWfuRJkNGQIammSf18jPasP5\n"
"YHyr/LifAkEAxoJ8R8Vusexo9ZjuU44qXCSvJQ26UBV7mn6TGEAn2DRK1RWKDaHv\n"
"j2vnRjt3CO9WPDQL7SB/1HNAy+dIMPyqywJBAIB6tESIz8zPniX+TJ18UKMTZwXP\n"
"3YQMvVKpUdDRLjq+OBMtFizSRD9MJOlUzGvibUfkzTPcHRDcyNbUMj4vbIkCQBx4\n"
"6sqAjvgGKKfRX52sbnb47AYsieSisC/gp8h6qzxfg7w8cqix6WJw36M7ND+b1Iqe\n"
"DHfeiXc3cLvOWJRuKTECQCEYkujtSjXWb26xaESFWGtUI/nEvCyqYPQAFBpaGzQ3\n"
"tiTDeKHzypesWYoTxOiNQWCQMLrFGuUbDpYOuDOVNjw=\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXQIBAAKBgQCcwSAfytnspSSDX/sKmCPOMnpuCYeWA4wbz1wLyb63a8/KXhhG\n"
"6o2W0kt3x1vnGZkeWwZOeBFUqwoc+xHhoNcZFsMOyqbqA3UMZW5cx27MsexRTQHs\n"
"Go1newu/E+8NNCohY51G7z1Hdo0L6mi/Tldh7puuGsMwKqNG/Vvo/GQDgwIDAQAB\n"
"AoGBAIUdpBAbjXDe1OET0vYuOMnUKA/l29RS8tpy/zGrg1/0GCM8QNWIPfEEaL4w\n"
"+CSKonMazYI5iE4kaZQuygKXOdFqKxX8nrGK2hR0DIEUHhhiqyGMUKrf4ELkAJzK\n"
"tHtcO64OFEU2EGa72wCmyk2MhqhLxWxA7E00x24uvW6pen6xAkEAzHhbzlRgLZ+K\n"
"QuXmQHEqkGaS2Ccf6c9TA5Bf5S2/5zBl+OqVyJJQH0yrbPYR6Nn1NeSv3R4IDJYg\n"
"fSZLaVzWHQJBAMRCU6QtTnZoQ97pLvXCSKRYKJF+CnE3zDFTyoJrpK0W1FSnb1EE\n"
"DWjjdSdMLynf/InX+VOaLk3Gxwjme4NKjh8CQQCg2b4/HplayrsVzY3I/D2jw02Z\n"
"xY2RfYusrhMCU284DBbsLn8OfiuRs9rXqOyF5ZDFiNXgeROT8zYzvcBtbp7xAkBU\n"
"ZET9IvJLXjhZISItUXbVHIeNUIqC9sBaMbKx9EGioF97a2gliT2O7cgRtuPM+ODq\n"
"ETHILlNc5G3vuNRBt4x3AkBV98Y1SZA3TQlUVTsjGraxkFTfU1IlomiOdOwTQ+xZ\n"
"x+JxhhgZwZ+kgI3PidEufFCTZJ3WO6Wk9gk18Bx7CLjm\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXgIBAAKBgQDq/K7wNW3fcTbaRTjNZlM4W0G7tKeO+X0bca4+9uin3ML3ogNJ\n"
"6qT/B0QAZB6Vyi9kKa3E8plQkjmPuX8Q27zj2QjEuDZ12RGFnikeOosUhOYiDh3Z\n"
"T9CHnr6stozzgk79Xd6VI7bqRcgRwbY0uc9QVr6vwddyIfSploSpVcgspQIDAQAB\n"
"AoGBAJfUpo/sZc6uzxtfCKGmkPTj+ef3hSBbUZuu60AhtxfnC06HrwpOg0eJAUYj\n"
"aqOsHMziJTYQ7kDiCjE0UMaqxDNS5hueumznq2xM2mSN0nYoktU00kpANVkW4VPA\n"
"33TB16DyqlKq2/21Rs1g8/8+IKkKDbRLTC//1WqNHASQVoGNAkEA/+z4hxTVXZkr\n"
"9hz29tAHKURlqzxUEKLnS0eL+XGJRNfGJ+65eXL+gFiIbTnpVeidL1+lKWkZyYzl\n"
"75cNRdUHhwJBAOsOJ9mUOqTbLW5tzh18ewZGOa1JcxhOvf2E1d56N8tDK6lvoqkF\n"
"oUUb8kIweDxPLCVLCl8qFrbjn619fxDInXMCQAfEZGKNIlCd5nSoumIRPDZnagKB\n"
"aTe8CfMB7+CZLoZVWiE6IIzsDYdNqI5QFKHT1nlqmLOiCfNRAGV+GxwEdB8CQQDE\n"
"sHu4HclU2fMSTOAE3H01qt3om2WsGXfyBI3SNQMrG3IVvkymkwd4BQKbUGPMU5Pl\n"
"QP3U1CtdruuXCUSijrzxAkEAoqYub6+0zM8fakSQZcZ01TG9Fuo2xVFDCQsvqR3m\n"
"ZhRT/oinIvOxSh4fQs40bmt1RBmc2L1Is6YB2NTVQEBZDQ==\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICWwIBAAKBgQCrf0rPvHYaGYQrc1ciRwaONs8TUvSVmUU98HMYXoFEkBL4CAGH\n"
"4oNHFk8kXHEOsBED0eccSYegWhqKHSz7PbjmJaXloExWrtx5ea3Twf8VTgcfDWQP\n"
"0TzD3G1TYjAFPQ1/LAZCpQFmwpMmTGGxegUhOzkpEWXdLVEVc9Uw4C4L2QIDAQAB\n"
"AoGAZXAJZA5pHM7y6nBynYe9TOkGWru6h7H8zsImkcd0VoWRcrvpi+JjG+0KKsuy\n"
"46kop0XEmWq0mhgxknfnX0QG1MKTqGMIUGN4qCaezOabIpCOdA4d/pr/mWoNgOWw\n"
"9Kc/tNCrKxPKsQMAlWP6ktHN30XRSlHgAjSeUVUiNHztvTECQQDUNin2nyIvj8ZA\n"
"QAsFW9qW+TiTkeUK6yiZ9Gvgf20gwZRWOe5/xnMxVvtN6v7Av1ew/l4VhBoj/w5g\n"
"ydIZk+2LAkEAzuJwdt+ccllG19qmEcbo9XFafgi2PvlEjPJmT1rHV2ns/7HIMu27\n"
"PJY36GgExSfFco6VmicaoOt+RKg+5acgqwJBAKQxAEjcGWQ5VsgRhTVxO3DChX7Q\n"
"TColhrWPwwPhM/s7K92HVzwvvKL5TNmdr9xMb7n3Ja56FouxZVuH6/J0XT8CQAat\n"
"Mhnz/3WFQg8HRGLAe5YoMVZt64u+uaKe1ARtlo9QoNBjqWVTXL6IzocWjEjcjrey\n"
"uEtARdC5qNqIX3dD3H8CP3pVCPvpHOTxkUaktmLYowSA1HSfO9wkE6bMCHhkLwXF\n"
"yTIJ+N7c5u5YN1B6hhVqpKbdnSv+K0MQ0xbfwOWNMw==\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXAIBAAKBgQDGQmrKfO3WovoXkOTSh/shO9qjbX4izhg4pccVU3Tp45v/dgAE\n"
"uDUuaa/clToyH5AhOtuazO/asC3ZNajg1ia5VPzmQU3gtqiIZIEXFaOovPlOrXru\n"
"wyQnxaGORndJwfDXicG6bUwI+PDpNq8c4VOTujReeF0r74qMSc7TQLVlUQIDAQAB\n"
"AoGAakR/aTm9YibJVohbnl00xoOGlcLCsXU2lmaFZ3DsYdGWdD+TkvQJzW7ozJtQ\n"
"Lj2sy6L4wujGR7nXWW3hr2IaLpoc1UoyJpieAZM5os6bMN+N4MCqdcZMlazMtSWV\n"
"UDO7O7xQGFpcvvZmnfKCyluFaJ5K/tWxP+2TnS1/m0BDRIECQQD5DYvToA0eKBt+\n"
"7K4eEI8pzDot9NlcL21D86kNgpmuY4pifALU7GvXr299JpFFiYa2A1JVRfpQaoI3\n"
"hZzz0ze1AkEAy8opWJP+T2q4reD5Qq5UjjrHUXFID23KeJEjh5YF40/bHqyVpWVR\n"
"UMntNgAzs+13vRij48Zn6I8GRhStaQ3ArQJASPyFS8GN1paeaDXoWPs1WWR2cF1f\n"
"DbsAZHeVxVXOv+J//ZimI8wdVpodLCoPTLee+NxEVqUpVEPCYY8QjgwKOQJAATmj\n"
"6f5pxvxzQ8hYd0gpBfngfOLbdgxI7VSiDAyg2G8AeDy9YZMsW/n6zRpPNUO2NpLR\n"
"WWs18LX7aaxyJnGIuQJBAPPfy9pd4XEFsRBIIe3N23Gua1XkS/407RJtAGm73Vrt\n"
"QhtWh3i6D5gfpEApMoaE8aaQQ7H0z+0Uh1t8SWesy10=\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICWwIBAAKBgQCc/M/X8etUqrxnmH3PyuAYLIPZhwNySch8qz9NB47izYjxzuBG\n"
"GSls6H7WeKIrB8UJY1gW8TLkdOLcrI/0hTANNHEPaueOE0xdABFj7tAaiiGPIM25\n"
"N0wc76me0ZAMYJrZTHk8JZK153y9wInYBwVZreXCVSVf11RuVwe+iFQa5QIDAQAB\n"
"AoGAQC4XJtivdhDLL6snHFF7pkZkrQTGgu3pOhakrXA+mTigGQOTqvTUe8LdP/9X\n"
"hTIK+tiTheWcAcxLhx5BSB0/VDKjYhS0ROpTc33Iq9KalOQaTJbBYGA4eagpQjwU\n"
"jGwr9u2sUsM9WI/Jg0VvLSKhfnNwYIUzLpK3BbWb2qAdh+0CQQDQ2s/8DlibFSBK\n"
"UsFK7lLpV8UgMk9CkaNM2BPzI8Hsjpp6s3pULVRd36m4YTSg15EEHv7bZ1N/+krX\n"
"mXb9xUULAkEAwGy5wHsUSjTK+kntkNXjlCU/+9R+HFpzg9Bwm/PqXTBwEWeU24hV\n"
"iRjPvqPtWFZrWi/nfcviuMaqtdliw1I1zwJAZ2mQxhtMYC2LuYFUWAe9YfClmJWQ\n"
"jUOTef8bka5I3RqW/t5TWc7AEWMnpDXtWx6hnUrDolt9Cschu7MvKeQ9lQJAL18U\n"
"46PpPNN+XNuyVoOxgRkihVasrUI/SeYYsuv7eHGiRUagyOLpW9T139LvbV3pE8zT\n"
"So7VA/Q0towL2lX01QJAGcoBNNouSpum9+5NvGQK1XXsZweawE+pFR2BE5XcjG+n\n"
"FnaLEUBX7nTxhTU2cSQET1PKRNp568a281NEna0nxw==\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXAIBAAKBgQDFOqqGG/VtIScxayZYZ+BT+hcs5W1bD5qRxunbG9O36UVT18UE\n"
"CWw9HUf0Q5sDMGvVmBxwZ4GjbR5FDPfhIXaRCzobnejJXq/0k+O5NAVkcSPtJvhK\n"
"AaUqBrWA41vnjKOtJudTsZLfufKafzYwVonze7fXGyVsBRjVwHNS4iqq2QIDAQAB\n"
"AoGAJCoStI6R3RXUKvKb0GATuTJFZ50WBTmCPTK9FMkwdCuY47vPy2Ky7y3cUMTI\n"
"urf5PewrYs0H72CFyWGMXkKVi8aOYshsATEXMfGSqOcqXn+UDssRzvabZFlpnAUa\n"
"WDVt/iN092AdakXNna7/DxrLisDpq8HHJfjtlWGPfkXRg4ECQQDpHeKimTvwJcPc\n"
"iDa6Qb/n9gwLeRckfzhYtfX1luJYLIOHh+J9vjQN75thenBLQB/B6qlKtOn9ejxg\n"
"5z+3zIOpAkEA2JbxXVTCOA802p9khvHxDtLHdKi3w/BjjJiC7Mgqo69ZI+s3PB9E\n"
"F2HJA69kZqpGqvybWHDapjWsq7rcMlxrsQJBAME2yvR3y00VEAyGPc4M1vF8ZqlP\n"
"uRW/+ETWtEDUyU/JvU6lGt2bu2tdkEyv/cjxIiFIzP4litdT7B1pLc+6S9kCQBwE\n"
"usiWFGHoJbA6emiyl7qRLdg7kzo3uMkRWa6D3nA6WM+6t/SBHu/faH+fit91G5s2\n"
"/mmcf8yMmP/GNoIVTqECQFl4Pt6yGiz/YVoYSp35ljY5n3JB6T8o2pOmIrRLuPmT\n"
"6kgyygtJBAmx5nnQoeG8n08tl9QakWznKzkNJ0DIFKI=\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXQIBAAKBgQDCaOqJ0lsSAEBcnNB6X7BvVcEcol+evi/nJsPe0uT1SbtW50Ch\n"
"vYOHwK6aQR2C5x9VSs47cLynTL7tNt5d8oeryF3NpI8VTPLImDJCcvUZhS7p4bxn\n"
"JO+Wm+D/e3TWfyjreuWtdL+Mfimw2gzwWuBEtmj51GzQ89eYm7fh11SB6QIDAQAB\n"
"AoGAWaakMbZNxPlUtOCjyysBY/Y5vYira7rswD3CKak7aFn+CE9QIMYSN7IFUqEg\n"
"iNMoQd7jR8nvVX8wtJeO5+gF48W13C3n8FZSrW7c5N3bmfMIgo0xa/TGfeXHP98o\n"
"7vhH0I58j3ZZt0Q+3wTm7t7WPE/nJzgrCk30TqmoaEmstTkCQQDtV6YZ6juEK2Lp\n"
"LGUiqohcS/WJxvFrF5+LNpk86Xdgomf6FphZlkq42KYkvl7qibKDcfDqLKTbHHle\n"
"vQQeCgZ7AkEA0bFHi7F8o4iHtKleBvt4QCj1neA0q3CRDypCI5EqFSrNpxY4Krhh\n"
"WYSVX+xT00QYaCpKKWfYQztCw7Anylv96wJACl86Mwe5ch0zRV1bThiFvQLUyCCZ\n"
"jESMBFlueOr6/I4cXSF/puqaeVl+aTyoiTdbRcNE8/bffXPRGgLIm0d04QJBAJSY\n"
"lmTN789Lby99Xh6AkaSV4ghw26Ip8QHYJmph8npxjK69Niw/4Oy44cnKBVUPSmR2\n"
"o3tYFY7/Lb7S1D+4lOUCQQDbMQUGVsZT+ZjuOG1bAjIuXoAOfOd3mgH5VgQHjSgJ\n"
"ourZtlJ4OUpNrq9IfWqPkM+zSE8+0Dk8/9MS5ngBA/SJ\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXQIBAAKBgQDNbHjwg+7tVNr9erMLowXRnIcttp4pUJbr3B7Jo/u+kD/Yo3F3\n"
"4rIKhHpJl1uEHP1QmvAD+4ApFFI2hNG54xYI8dGflxL5HOs5xxyOPpkrwzQ8Qvnv\n"
"LPg7Gf6PAW9zF4McG4wK0TkrV28G6NhqcPs5VFY6UyvfZ0fEdWAeoWTIfQIDAQAB\n"
"AoGBAKOmkMp7MLLd8QAS6eSRYSdWHdLrMyES1MjduaFGBF4SKOr7en/Zl6ENXSaX\n"
"cA7V0XCPnjpt9/HCAKTyNupx4LCeFWiqdu8VGXhlzX8bdb896OSR2brKbxgRY5tF\n"
"36uL8akrZdrYgocykQCxmRARMB7/rHwDusiamjL6RUZ3+c45AkEA6UPTVmKZQRMr\n"
"A7Qgg5nXrXo9117Lpqf3FdZ1wdni9V59Ptf5xrx9oGZNZzctJPXSAH4M4cumSJrV\n"
"sZ1V8qE7AwJBAOFx+5luLrVKrdlG7MyOhTAdhKYUvKIvL4wvVSY6y+L2nNEx/cTx\n"
"KYbxGC+H1RJbkCS09rYir3VfDRWQ3W1c1n8CQH+X4hn2hO3blkPIW6CgniD+JKWR\n"
"7MOUTMtdK7yFemfM76VYbgAPSohabSxwOfllnSE30cQQqTw9tXYaIdE98BECQG+M\n"
"QWxSS0QillB6unIgVqBPCrJOcmNhK4qWZPBMiVNcqI0Nyj2nAeAl7MyfzfqOWY0A\n"
"CU5nbR+LD2NLUXRqSisCQQCN3IGv1WOWInmA5xhU6vCFDX5u48Dcji7VLJO/Nv/i\n"
"b/zHKAgjHk5Js7bi5ZWEGaUgA4Jt6cKmGdERheqTMKxx\n"
"-----END RSA PRIVATE KEY-----\n"
};

static const char *PREGEN_KEYS_2048[] = {
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEAoksI1qIuIaFCqT4QbgDvOQCmr9Z9F0E7ku+U5Ep/5dWNANqB\n"
"bSzAOq0+cxiisfF+H4desoqiWDUwlOwXH74qD3ZsbChhvFUD78cQBWQkF+whLVHb\n"
"296QmF0LZqosqz9HMS9CdoMUc1brZb78Hb25QIOOjrg25KYHLZHaqcet1wfhHow6\n"
"Uehc6QTuWgOWFhJnfiXzYgen2o8lnLixxZozhk7Lm7Aix9ur2ckXdQ2Wgny4xw70\n"
"JW84Hapnd8oFUD98XXrExk4VFuIcA8qo7r7y18II6wx4Cw1suKru6bhW65cM/y51\n"
"KC4lB7VkvuoJCelRFdM1PfKZLv2tJP63oAqJrQIDAQABAoIBAQCWc38PEqw3avqU\n"
"UMAEaoNa0bq1Gd8/Nq8WqVnbRSFKHO2pk+cWIb1W6BITuwvgcGKesezdEV4s7apK\n"
"9I7/U1hEm2Ep50mrwRh0KZM1nD9Fmharn851Bt//D4qpMytT2caS1yADI8NKpZJ1\n"
"8VZh7+cT4qG+txHUaAIRgbw3VrBWvTIMu6SOSOZm+e3eOr5UU3du1KvjdJHJ2c2k\n"
"TceHvUdKxV7OYt+BBSN1oBOhs3ajUSRge1v3twRDg3cmbwG0DeXvwHNhGUTcF8IH\n"
"JO1RF5njbkFvyqdAi3ltjU41zYd4OMuPtrwzFOtxUjKT62Soz109HUXXE2CGKFPZ\n"
"PVi5/BIhAoGBANN1xqS5BgHszIB0nXbw5ImYpTRmyhO0KsTblBT9+8Q/B7BCK7bM\n"
"zl+dOPeyvEadSwE7RSMMt6CAlTakWIf3Quw/VZajvXy9C9/LHf52pEKXjxMFMPKE\n"
"aGLHpQnwMtDi8/H8AEAXxI3hpxB2KVR7sAYHWihSGjRJ6oPGvEmKEkb5AoGBAMR6\n"
"G2PKz0xk1vFrjfjSY+y13gH/t7xHaXUggjggUSGKaknQh2BDUllXjadeI0fi1eLW\n"
"r98ZImZZgntAgjaIZ4bAlooTDk4gRHaz9jI+z8lsRwOKnWdiigM7txiXZTMVwMqj\n"
"o5mMNGMA+A+ACkTViRHmkDI7S/9FqAvnbOqVwgFVAoGBALUcY6WDvwx5B3Jh7tgH\n"
"XIYpEh3+h8c2gYcX1g3gtvkPTwN8uToY0gz8eOVV1YHZiHsmi4GIi+HRH3usaRMT\n"
"COOVHzYlSc8Dj57+tdLTRL6wVl9hC9o647ju64DGlI9qQquYPZKniLZIdbFYsu9j\n"
"/JA9Tc/I+h6czFpPJccKlbrpAoGAAPWXrKUQ3g6f/g3IY66jTkSVEO1uuDyhBzFh\n"
"cWS3ALLsUe/yuUWa4VTMHEUZZwB0iucBdNVqlZVaTb/C4wFHgCDwmzv8leUScIHw\n"
"cc5ctV8R+bJzkk2o3tsrybLzi4xPpK2n3tgQaWtXyruVUUC5qpy1l4kylcyBRY2b\n"
"uomAqQECgYAiCNWtuWIDlRBcvtIB+kHguzcoFT3vTCCNhalTEn0zi/tbi+voQgVJ\n"
"SDJNptZv+6vRwQ/HfcQtljKIPO6hUZPYaFWRNhgbh7Ay85lRXYXQOottE8ayReBk\n"
"zZb0fl853Qah4DPsaOugAvhjjKeBmKg6bFWO1z6hj18I3UpDf2YnVQ==\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpQIBAAKCAQEAssO0r37mSJNAkc/ISwXBsu9JjyLeWlsHPAhylQGkSAdp2rjz\n"
"E6AT0Eh3wrocNO31I4pvHReAuh1QedGY6T1cQwO/WAAhQtRCBQDK12qWRgfbC11y\n"
"Xu7zNYPd1Z7YIRy+FxhbL5f+lv3rEUv0HUG5c3CWhLtbANKg+jOieIDzA4Yp1s55\n"
"ynodQBUkTZrwQiT0P8yDSjiasf+clgJRfA1k2XK12KSAMRgyDuPTE4OtBxBvUM3L\n"
"Zvxs81PsmcOuAG4DLaFTg2a/QkCjt2VC1SYYuh/LVxpL41FFh3eMoK5g5deHkgRe\n"
"tlywKjAHIDJu/qgNzNgNW7ymwn2CfBvry9h0/wIDAQABAoIBAEMZ4wDdCWPEokAZ\n"
"Vn2Ss5qO53WrCPuxn42RPjFgZGIFJl7LfbKoK8fK6+lUIrJbf+DPXdX1tIQn7MVN\n"
"P7CNL8yX44MMyW9kbUOjgIBLqgyvdjFV6lBoMTKtRN+iuE31lATnR5Md4pqaxVnA\n"
"wOkaepoycM1x5j7w0SwZparF/HIdkYv0y/MysqT9ByupPA4Fqp/iRSrosHXahNtI\n"
"KZYj1TyERYtuDXq91P4dr/pWq3FmDNI8O3upblkL0YouvG/ZlFLdiNy77XbAyWcX\n"
"ps3YDddM+vECnXO3+sa3ZxgBYvXJdWrrIzM5A+jCkDRZQGsFAzK5I5/S7C2ljt6i\n"
"SmzqvMECgYEA16bGy2XTi6KBPb8aev/OBgK9XuGLwUqK1m15mS9Y2qPHmuc22qaZ\n"
"hw6zginPFrxAEtQWKanhZy4aVqlLkDPLwRnyeuMo1EZAc5B1gZ5ViSAKxBq99hA9\n"
"eqyakdb+IUQsEnRDxSc2gqUQ0EagksUyw5wGG5Q/CVEALmS/r1SU3KUCgYEA1DYf\n"
"6JYdzuRtule3vYeWXKf8sOJpdplgWV7tvLrKkQhdE564uwMCYB23HvYfwWqEdDYG\n"
"fsYg/ur/stk9MDZ3wZKffTEM8V3sX1t1JXnC3ogSAgMGhLZ3ILOLqkoO4BEZJnsS\n"
"dMdiNijlAtQkqs/BO/UVUAKysCtKP3v/+1775dMCgYEAvLjGFjApfnSbV/cK7IM6\n"
"wEXbhdIqZOCgOeEaXjVyM/zKbMRVW+oaR3hVHd8KzSG3jQKv1oxFpu9Qu3ByoWLC\n"
"uF3Ft0debs6ADuJoAyQWROeWpGGmxlUWCGpO5rxYL7KiQxAeUsXrTU+5NBvq4CbV\n"
"MxwyuCX3OGb7mp4upfiGQcUCgYEAuhVsDYv1P4LXJVvd5viKRV2ZG5KuYC1Ga5fu\n"
"aFxzXJI07At2eaa94oKsHR494mEBHNZzA5/BN0fiSHZuTWS1xqxH5oOokc6Gg2ez\n"
"ZdVLp88x20nD4YQPGkHW6tBeEuVrZG7vVC+yU0Ow7bYRISdkjqrusWZsQkbzqI+X\n"
"fFliEbkCgYEAu8x+47M1ordbI7NmbBGyiyP0r7nMRCZ+KEvGeCNYracWmsnCNnfV\n"
"zR2UzmwtSainw3Ho8Jv/rWDC8RIDauyBRYEi2VqOnUzT2ca0iymQyLeBCudAQuio\n"
"drOu4JU8RzZ3Ad6V3DNFnaqmX/7GA9Pa2GI8NJMyb8p1GAGv7Gi8nxc=\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEowIBAAKCAQEAt01S8JuEwWy/Hzb90yO2O7oGWq3GfvfDpFOF4OQnwG3kQ/BP\n"
"4MoPDCYHdqb3iI9aD3vykZA6Q8zpdfGwjm4+bHrgRdiSmZWv8NvRwuQ5Ji9xbiGn\n"
"hA1XwqH9hvgFTiy6tRvirWSJ7kzH3Q/bEGpCbHUQkwMog4v6yCNKNrjlwjN++eCi\n"
"gFK/0RMOJMLOs8BD3zY+lKjd/pd8LBRujkMyUF5SryeRueAFjD2sq4OXq8DPABGt\n"
"zdR6vbTcsi4JwP1Q6y4x0/LIWEprzzewNU63I5E2zj0WnoRGAIM4aF+VuqcHjWUx\n"
"VWnyLZldSen6lScZ4xj4seitiDbSFvtFkDF6VwIDAQABAoIBAGTP9im2ntDyyjqU\n"
"uA0DuxomOZBtupniEouyFBOX5/UBe2WSKZxsBNKdp8UuFz3X+aRCeyprtF/NtyjT\n"
"AFOVdmebPPWtIxOtK9LAUyFo+7VwqmXzxHnwDLBS/2jXx7MzDozFBWpvvRx+xf1i\n"
"1wy0JEwaJj90oTeYKRkhr5NhJZwkX8zCNYaemBd3kHB3aGWGJasI1Y81UezeRKCn\n"
"hSbn2CrWalI7pyJ4lsavM11nIq1Eu2ZthJiNCMghbYrHoBHd+iVWiCYchP2rNEWV\n"
"sdHtaVHtQ9zdZ43bao3OzPu7lAjd6UAbxsuhUe+a2YdDz/+Up+6+BvQf1FCfYIjW\n"
"KFUdCoECgYEA4t5O+u0V9gkMUhKsevYb0zgc7O/mo8ivN+V++EpAtL0mhiwxeO8p\n"
"oef0szLyhdULQeLN9pJQDCeAbkGdwIe3L+AKU8o8BFGEWLFysZjMg9In/UTrp5MN\n"
"mMDy2SRKKu5BqsvdYH302xpZfHq1T2cMNDWE8lrZffduH06Cgq/XEtECgYEAztbj\n"
"bhFneADnrvk609VnOQvoQEjySeCQKFQFRRI6k/FguqMisL2IRXnMaWammosdeCAg\n"
"m7eZchnszHIst9cwZUKXUFqmAqeDuWSNdTI7uKZH6nT/A6IDlgdjaHsqhvpK0Ac9\n"
"ngycdHONitOZh0ZG74pdWjf828Dwzf+CuYjl9KcCgYEAmIvI6ZqvkJ8m5Kzfw1Jn\n"
"BVCOypbJK8oOX3R2Orea6KzjEYb3wQx3nwFcHX6danYFOskpmqlpH7MT/Y8rZsEa\n"
"4RsxdoPedTzm08iFiXtn0R9nejp0hlov402iPXXUVSedih3IflBTa1w9XaEY9wog\n"
"P57ZBSknYzcTmgNtaDiaUnECgYA5sWauhNw/dMEq5QmrnJK2LsQRakdqo+CR3x25\n"
"LmR4b5Nze51pfvRLrLV/kMpXwQXvQ8bUqFl8og6S2CXxAWzWUcSy/RXhF6h+RbXP\n"
"Qru1vWvB0fBvqvklF9p6giBSle3YKKzfMNVTBggs+OiR+uA+YHG5gHRfN2nzi5mC\n"
"9tRtcQKBgBnDSi4lRCjRe9pPnyAYaa4iyBUGhjPysScSLY9orel89+qmTBQ/Py6J\n"
"0+sefL4ZJaOsuaR2mSSPP/lbSkF9DMFs4tHbBqY+WkVNYLshAkauHwqv26HTVCSd\n"
"QKzeb7uZw9lNaRIzDvy/3wfCLvXfdDozPFrOUgkyaBN5pJSA/4sv\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEogIBAAKCAQEA9qtiDoJWqU/eSlpj381eG6UcDzfMguFh/q4e4s7QVdRYj5J0\n"
"Msv0PCkti8JHuvQUyncRpOPccBkhNbVjNbjIgw1pHaIZNdVotUDhP0kseRyJ6z3M\n"
"qbZ5qKn+0mHjVjPNItVDDe6tebYMT1BZpVyRrCOqY2v5z1ecLC+ReygmHgDpzg+L\n"
"0rWfIxGT10IPZ8pAlcdEn6xt5aEhi7mPCX/xwqfQChPIJz6zVLEC8UaPtvDBohPR\n"
"6NQTBTeZZAAtzrQ7+oNxfz1v6Fz6RwMei7Q+qOBnMiwpQmbcDBKABM2RnXSpD0LA\n"
"1GR7/+CiV1HQoShWVvEwrSIlM6jVAJo6iqF6WQIDAQABAoIBAHqwcdxPnfUm4aTP\n"
"4r9NcZKEhDlZgqJSoiA/0OL1BRC7xrTanmspoLhPrvTF1FG715+Aq8j9AQbMqQUC\n"
"zG7LEwiEIhV4K9vn4uXMeHy206UFud/E5EhBl695pmJUB/Q3XcAGnQyP+77++o50\n"
"o7IpIdeiAbzj1uP3aplbq5u7M4JV7fUZWA/368G4HolqFTxcAfBJ05GXlp97BBwY\n"
"AnY3/pNrKMz0NiPf3nsJHYWK18up0JCLPL3tomc94wuNZ66spIazHIL9aaKY0q3V\n"
"LkBrelndfYM1m4xRTnSOy6STu0qKTPOpX0C8XBLYs6uiXjRsChqSYwndCCeASaH3\n"
"LGNIcbUCgYEA/m4qvt8tdT4wEvnE+QUxEELmBtT4UFa3NnQISrzNlhNeI0Zd2xlp\n"
"SG0/pcw83mG2uX+V5xSaWL5LYfLBkvy83Y0yIWgYbbIkyyCOUZnTpwaDGU/FjWip\n"
"3TfXf5qpAgiez94sV+MsFpKfG05yxJh5u+3sIyGTVUAxp0HPx4LVgbMCgYEA+DD1\n"
"fu6ttpuV1UMrsFdjuk6gBvSbyJ9OilY2jT+yE7hSRc/yP3O9ikuR74tNlVrWTnO2\n"
"0kcYbyLJXE2cGUC2q5e4r8TDGiozNfQ7/OC2M3XaJ+xJk4zMf/8PuDDpWr+18ZXA\n"
"Pf+ibXWTFvZ6ZeUmpbrrfCrXdvmIZnwVuOI0FcMCgYAZn26emksxq3mb75tumJ9A\n"
"S/xuY7Q+Iv2Adl7/Z9QscPbiBowdLIn1yUrHn7Hhk2WbeMXX57NDjKZ6zr+/1cQP\n"
"a9DInHsZUP9zlWu/vAYcpAM/4VC71PaGWMFTEHhExCl6NZ2xnCcsfseXMGdOdSyN\n"
"SICnaRI1W6mkdnQ+W2a1EQKBgGEKA3KVr6XuPy8bDEHuaTe29irCCQbwAq1j+ABS\n"
"HzZGoyRYocbdYgZoda7LMJJs6c3SwHCHC66oU0KbtaTKAKImuDdBH2djiJJX4/yD\n"
"f7mvIpTpdfsS2gJRn7vMo/CvdFv4ySl0gfV6OwCHbmPYrLuv0dLCjWwfNI2dhoC7\n"
"MNIxAoGAIPSIG4BrShzbeX4c2L18iwIg+NlOcUbtl0Ccr1t6uLGI+ge/6I6T/5XH\n"
"DPKqYIf0IRYV8suxpfQNKiz/C0NPffA1d1M2hvuAg2v09o2cSwvdcQwdmakKZ5bl\n"
"sdCuYKdCIwomEUOz/4XgQrJl4XDUqxftJT6/egAjWvcIYvfNCsY=\n"
"-----END RSA PRIVATE KEY-----\n",

"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEowIBAAKCAQEA1yHZMsgRLckL+v6rgpGq9qmxVBNDxeuul1V/QlFyOlcAk5n/\n"
"uduTalSqGQhc4NEePMxq6nFui4ucpkZOozmcEnhV0N9jld9IB9rLGt4erdg7RKl9\n"
"+gQ+zTn69j69U36E2I47H4dM69uxeSOyWP2Odxpw+biisa3o8mMz1zCmuj4GMDtG\n"
"DlnSpthFzgQR6N1pbvxLXrWg5F16GqFiJOD7kXDfy4/l6kB/mDs1T/3r8kav6DqR\n"
"c/t3aQZxgWGIpI7hc9Qgvp7coZRMey5dNOZEna3tqS8dn2tZlhkpYV5uyFUjmxjG\n"
"TERSULQ7hvUqW+eshGGsnxFtL7ANnTSc4xECowIDAQABAoIBAFhJJMhpQFuIySjd\n"
"AGeZ/g4x/3rgWQzNNp4WUR5XLEhy0eLA7ShJywp06kVRoEQGraEHxsyldldAGS5H\n"
"ZhgoGTufNKB+PHER646FpJpHE1IGjfQUloVW3qr8I1iQ0MOGBWCVpf+/V7rnMsLi\n"
"+lr421FXgYuJ0QKXuyRVv72M0q9U6i+ml3aVAhgW/19oFg+dW7YccX+9iVyD05Q5\n"
"KR64tX8xd4wrAqfAgYA3erbbE6GTyHYD5K54kIgfRr/+pIU4qc1L7XOCblnqc/rI\n"
"BilFysEC634r2MNe66uQvNui4oQTfBcFFlXg0zAmp7d5QE0ApOL6HpCsmbImm2uJ\n"
"sdFNYyECgYEA716kfEv7HfnF0P3pAP2AOuEsW6t8q0UtWvnHrwRQXQw8Yv90g7kD\n"
"pUV3/BjD9VQgsQZosbdSn5wbT4j7dypRdrzYk+8m/hBk4Q8M/tWoRGVOn46NudvK\n"
"/KX0A4ODLuulj8yAZVc7CM5Cdy4GCGJBVO+oVvBUAnHxfZziOyqBw9MCgYEA5hQg\n"
"HEORzdxvbbfAx1ggvH1Eg1lqRhmpI43PpRkaoqb8jLwXb2CyBeuv3RBft/X2Tr6F\n"
"mHpe0U1kN/5YEjii/Q/jUX8azIHaUNNSAjrriEeMQZOqFxmhCdiyeXuqg2fbFbhe\n"
"K3Q6/fsB1xj9OOSwyPMqm/M5U0LsoGjmg8TFE/ECgYAlImKUIdlwOgp1NJ7MF4eo\n"
"Gryd8AmkLFQv8+YFgb7R4I8RsJ2rva0SG6fUhScJTSbRL7RYNZ9swXP/L7oLL5Z5\n"
"vCxBLu22pmZv/7y9X/n9ulWrLRtRhQaFkV08mk9knQwPNeOJVTIEWLM49/vZmxyV\n"
"h6Ru8FOoGXMkUI1MLnj5HwKBgGJLkNhiacVYeuaWDa9c0EeXARFYvxWJ2wAMkvzG\n"
"9+ErlFQP+7ciyYvMAItidnJii8NilDLrfNzQwpNFf5zxQ3j4M7bapblfdMT5M10u\n"
"jPfhEWPm0VEjKvDI+p76HYQcd7YU2W6ZLqbZeRTLYUvQMFL5yGduBzyyJ+P0TR9Y\n"
"jpYRAoGBAM7vYGTprw4w2tTZPFICXVk1bQ0LO06oNRtwkiQTUT6UqPjWMFyvHnmN\n"
"11SVVBmRZ0RAk6e5eZLFX8WelJ4J4nSOGRcJheCtoEFlO7D1ewAUSbqWJ0pBqp2T\n"
"gV4oCS8LYe8zReVoYZJjuLwoHvxZzs/hUjc3SI2HRW2W/HQRPC25\n"
"-----END RSA PRIVATE KEY-----\n"
};

#define N_PREGEN_KEYS_1024 ARRAY_LENGTH(PREGEN_KEYS_1024)
static crypto_pk_t *pregen_keys_1024[N_PREGEN_KEYS_1024];
static int next_key_idx_1024;
#define N_PREGEN_KEYS_2048 ARRAY_LENGTH(PREGEN_KEYS_2048)
static crypto_pk_t *pregen_keys_2048[N_PREGEN_KEYS_2048];
static int next_key_idx_2048;
#endif /* defined(USE_PREGENERATED_RSA_KEYS) */

/** Generate and return a new keypair for use in unit tests.  If we're using
 * the key cache optimization, we might reuse keys. "idx" is ignored.
 * Our only guarantee is that we won't reuse a key till this function has been
 * called several times. The order in which keys are returned is slightly
 * randomized, so that tests that depend on a particular order will not be
 * reliable. */
static crypto_pk_t *
pk_generate_internal(int bits)
{
  tor_assertf(bits == 2048 || bits == 1024,
             "Wrong key size: %d", bits);

#ifdef USE_PREGENERATED_RSA_KEYS
  int *idxp;
  int n_pregen;
  crypto_pk_t **pregen_array;
  if (bits == 2048) {
    idxp = &next_key_idx_2048;
    n_pregen = N_PREGEN_KEYS_2048;
    pregen_array = pregen_keys_2048;
  } else {
    idxp = &next_key_idx_1024;
    n_pregen = N_PREGEN_KEYS_1024;
    pregen_array = pregen_keys_1024;
  }
  /* Either skip 1 or 2 keys. */
  *idxp += crypto_rand_int_range(1,3);
  *idxp %= n_pregen;
  return crypto_pk_dup_key(pregen_array[*idxp]);
#else /* !defined(USE_PREGENERATED_RSA_KEYS) */
  crypto_pk_t *result;
  int res;
  result = crypto_pk_new();
  res = crypto_pk_generate_key_with_bits__real(result, bits);
  tor_assert(!res);
  return result;
#endif /* defined(USE_PREGENERATED_RSA_KEYS) */
}

crypto_pk_t *
pk_generate(int idx)
{
  (void) idx;
  return pk_generate_internal(1024);
}

#ifdef USE_PREGENERATED_RSA_KEYS
static int
crypto_pk_generate_key_with_bits__get_cached(crypto_pk_t *env, int bits)
{
  if (bits == 1024 || bits == 2048)  {
    crypto_pk_t *newkey = pk_generate_internal(bits);
    crypto_pk_assign_private(env, newkey);
    crypto_pk_free(newkey);
  } else {
    return crypto_pk_generate_key_with_bits__real(env, bits);
  }
  return 0;
}
#endif /* defined(USE_PREGENERATED_RSA_KEYS) */

/** Free all storage used for the cached key optimization. */
void
free_pregenerated_keys(void)
{
#ifdef USE_PREGENERATED_RSA_KEYS
  unsigned idx;
  for (idx = 0; idx < N_PREGEN_KEYS_1024; ++idx) {
    if (pregen_keys_1024[idx]) {
      crypto_pk_free(pregen_keys_1024[idx]);
      pregen_keys_1024[idx] = NULL;
    }
  }
  for (idx = 0; idx < N_PREGEN_KEYS_2048; ++idx) {
    if (pregen_keys_2048[idx]) {
      crypto_pk_free(pregen_keys_2048[idx]);
      pregen_keys_2048[idx] = NULL;
    }
  }
#endif /* defined(USE_PREGENERATED_RSA_KEYS) */
}

void
init_pregenerated_keys(void)
{
#ifdef USE_PREGENERATED_RSA_KEYS
  const char *s;
  crypto_pk_t *pk;
  unsigned i;
  for (i = 0; i < N_PREGEN_KEYS_1024; ++i) {
    pk = pregen_keys_1024[i] = crypto_pk_new();
    s = PREGEN_KEYS_1024[i];
    int r = crypto_pk_read_private_key_from_string(pk, s, strlen(s));
    tor_assert(r == 0);
  }
  for (i = 0; i < N_PREGEN_KEYS_2048; ++i) {
    pk = pregen_keys_2048[i] = crypto_pk_new();
    s = PREGEN_KEYS_2048[i];
    int r = crypto_pk_read_private_key_from_string(pk, s, strlen(s));
    tor_assert(r == 0);
  }

  MOCK(crypto_pk_generate_key_with_bits,
       crypto_pk_generate_key_with_bits__get_cached);
#endif /* defined(USE_PREGENERATED_RSA_KEYS) */
}
