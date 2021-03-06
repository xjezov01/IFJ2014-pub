/*                @Project: IFJ14
-----------------------------------------------------
  @Author: Marek Bielik   xbieli05@stud.fit.vutbr.cz
  @Author: Filip Gulan    xgulan00@stud.fit.vutbr.cz
  @Author: Filip Ježovica xjezov01@stud.fit.vutbr.cz
  @Author: Luboš Matuška  xmatus29@stud.fit.vutbr.cz
  @Author: Eduard Rybár   xrybar04@stud.fit.vutbr.cz
-----------------------------------------------------
*/

/* IFJ: scanner */


/* hlavickovy subor */
#include "header.h"
#include "scanner.h"
#include "garbage.h"
/*FUNKCIE*/

/* klicova slova */
const char *klicova_slova[POCET_KLICOVYCH_SLOV] =
{
    "begin\0", "boolean\0", "do\0", "else\0", "end\0", "false\0", "forward\0", "function\0", "if\0", "integer\0", "readln\0", "real\0", "string\0", "then\0", "true\0", "var\0", "while\0", "write\0"
};

void vloz_znak_do_tokenu(int znak, int *i)
{
    if (( token.data = (char *) myrealloc(token.data, (*i) + 2)))
    {
        token.data[(*i) + 1] = '\0';
        token.data[(*i)] = znak;
        (*i)++;
    }
    else 
    {
		error = interni_chyba_interpretu; /* interni chyba prekladace */
         fprintf(stderr, "CHYBA ALOKACE : %d \n",error );
         trashDestroy(interni_chyba_interpretu);
	 }
}

void vrat_se_o_znak(int znak)
{
    if (!isspace(znak)) ungetc(znak, soubor);
    if (isprint(znak)) sloupec--;
}

tStav porovnej_rezervovana_slova(char *slovo)
{
    for (int i = S_KLIC_BEGIN; i < POCET_KLICOVYCH_SLOV; i++)
        if (!(strcmp(slovo, klicova_slova[i]))) return i;
    return S_IDENTIFIKATOR;
}

void inicializuj_token(void)
{
    token.stav = S_START;
    token.data = NULL;
    token.radek = radek;
    token.sloupec = sloupec;
}

void napln_token(tStav stav)
{
    token.stav = stav;
}


tToken get_token(void)
{
    tStav stav = S_START;
    int i = 0;
    int c;
    bool konec = false;
    error = vsechno_ok;

    //pre escape sekv
    char *ESCdata=NULL;
    int ESCi=0;

    inicializuj_token();

    while (!(konec))
    {
        c = getc(soubor);
        switch (stav)
        {
        case S_START:
        {
            token.radek = radek;
            token.sloupec = sloupec;

            if  ((isalpha(c)) || (c == '_'))    stav = S_IDENTIFIKATOR;
            else if (isdigit(c))            stav = S_INTEGER;
            else if (c == EOF)          stav = S_END_OF_FILE;
            else if     (c == '+')          stav = S_PLUS;
            else if (c == '-')          stav = S_MINUS;
            else if (c == '*')          stav = S_KRAT;
            else if (c == '/')          stav = S_DELENO;
            else if (c == '=')          stav = S_ROVNO;
            else if (c == '.')          stav = S_TECKA;
            else if (c == ':')          stav = S_DVOJTECKA;
            else if (c == ';')          stav = S_STREDNIK;
            else if (c == ',')          stav = S_CARKA;
            else if (c == '<')          stav = S_MENSI;
            else if (c == '{')
            {
                stav = S_LEVA_SLOZENA_ZAVORKA;
                break;
            }
            else if (c == '}')
            {
                stav = S_PRAVA_SLOZENA_ZAVORKA;
                break;
            }
            else if (c == '(')          stav = S_LEVA_ZAVORKA;
            else if (c == ')')          stav = S_PRAVA_ZAVORKA;
            else if (c == '>')          stav = S_VETSI;
            else if (c == '\'')
            {
                stav = S_RETEZEC;
                break;
            }
            else if (isspace(c))
            {
                stav = S_START;
                break;
            }
            else
            {
                stav = S_CHYBA;
                break;
            }
            vloz_znak_do_tokenu(tolower(c), &i);
            break;
        }

        case S_IDENTIFIKATOR:
        {
            if ((isalpha(c) || isdigit(c) || (c == '_')))
            {
                stav = S_IDENTIFIKATOR;
                vloz_znak_do_tokenu(tolower(c), &i);
            }
            else
            {
                token.stav = porovnej_rezervovana_slova(token.data);
                stav = S_END;
                vrat_se_o_znak((char) c);
            }
            break;
        }

        case S_INTEGER:
        {
            if (isdigit(c))
            {
                stav = S_INTEGER;
                vloz_znak_do_tokenu(c, &i);
            }
            else if (c == '.')
            {
                stav = S_DOUBLE_POMOCDES1;
                vloz_znak_do_tokenu(c, &i);
            }
            else if ((c == 'e') || (c == 'E'))
            {
                stav = S_DOUBLE_CELA_CAST_A_EXPONENT;
                vloz_znak_do_tokenu(c, &i);
            }
            else
            {
                napln_token(stav);
                stav = S_END;
                vrat_se_o_znak((char) c);
            }
            break;

        }

        case S_DOUBLE_POMOCDES1:
        {
            if (isdigit(c))
            {
                stav = S_DOUBLE_POMOCDES2;
                vloz_znak_do_tokenu(c, &i);
            }
            /*else if ((c == 'e') || (c == 'E'))
            {
            stav = S_DOUBLE_CELA_A_DESETINNA_CAST_A_EXPONENT;
            vloz_znak_do_tokenu(c, &i);
                 }*/
            else
            {
                napln_token(stav);
                stav = S_CHYBA;
                vrat_se_o_znak((char) c);
            }
            break;
        }

        case S_DOUBLE_POMOCDES2:
        {
            if (isdigit(c))
            {
                stav = S_DOUBLE_POMOCDES2;
                vloz_znak_do_tokenu(c, &i);
            }
            else if ((c == 'e') || (c == 'E'))
            {
                stav = S_DOUBLE_POMOCDES3;
                vloz_znak_do_tokenu(c, &i);
            }
            else
            {
                napln_token(stav);
                stav = S_DOUBLE;
                vrat_se_o_znak((char) c);
            }
            break;
        }

        case S_DOUBLE_POMOCDES3:
        {
            if (isdigit(c))
            {
                stav = S_DOUBLE_POMOCDES3;
                vloz_znak_do_tokenu(c, &i);
            }
            else if ((c == '+') || (c == '-'))
            {
                stav = S_DOUBLE_POMOCDES4;
                vloz_znak_do_tokenu(c, &i);
            }
            else
            {
                napln_token(stav);
                stav = S_DOUBLE;
                vrat_se_o_znak((char) c);
            }
            break;
        }

        case S_DOUBLE_POMOCDES4:
        {
            if (isdigit(c))
            {
                stav = S_DOUBLE;
                //vloz_znak_do_tokenu(c, &i);
            }
            else
            {
                napln_token(stav);
                stav = S_CHYBA;
                vrat_se_o_znak((char) c);
            }
        }


        case S_DOUBLE_CELA_CAST_A_EXPONENT:
        {
            if (isdigit(c))
            {
                stav = S_DOUBLE;
                vloz_znak_do_tokenu(c, &i);
            }
            else if ((c == '+') || (c == '-'))
            {
                stav = S_DOUBLE_POMOC;
                vloz_znak_do_tokenu(c, &i);
            }
            else
            {
                napln_token(stav);
                stav = S_CHYBA;
                vrat_se_o_znak((char) c);
            }
            break;
        }

        case S_DOUBLE_POMOC:
        {
            if (isdigit(c))
            {
                stav = S_DOUBLE;
                vloz_znak_do_tokenu(c, &i);
            }
            else
            {
                napln_token(stav);
                stav = S_CHYBA;
                vrat_se_o_znak((char) c);
            }
            break;
        }

   /*     case S_DOUBLE_CELA_A_DESETINNA_CAST_A_EXPONENT:
        {
            if (isdigit(c))
            {
                stav = S_DOUBLE;
                vloz_znak_do_tokenu(c, &i);
            }
            else if ((c == '+') || (c == '-'))
            {
                stav = S_DOUBLE_POMOC;
                vloz_znak_do_tokenu(c, &i);
            }
            else
            {
                napln_token(stav);
                stav = S_CHYBA;
                vrat_se_o_znak((char) c);
            }
            break;
        }*/

        case S_DOUBLE:
        {
            if (isdigit(c))
            {
                stav = S_DOUBLE;
                vloz_znak_do_tokenu(c, &i);
            }
            else
            {
                napln_token(stav);
                stav = S_END;
                vrat_se_o_znak((char) c);
            }
            break;
        }

        case S_MENSI:
        {
            if (c == '=')
            {
                stav = S_MENSI_NEBO_ROVNO;
                vloz_znak_do_tokenu(c, &i);
            }
            else if (c == '>')
            {
                stav = S_NEROVNO;
                vloz_znak_do_tokenu(c, &i);
            }
            else
            {
                napln_token(stav);
                stav = S_END;
                vrat_se_o_znak((char) c);
            }
            break;
        }

        case S_VETSI:
        {
            if (c == '=')
            {
                stav = S_VETSI_NEBO_ROVNO;
                vloz_znak_do_tokenu(c, &i);
            }
            else
            {
                napln_token(stav);
                stav = S_END;
                vrat_se_o_znak((char) c);
            }
            break;
        }

        case S_DVOJTECKA:
        {
            if (c == '=')
            {
                stav = S_PRIRAZENI;
                vloz_znak_do_tokenu(c, &i);
            }
            else
            {
                napln_token(stav);
                stav = S_END;
                vrat_se_o_znak((char) c);
            }
            break;
        }

        case S_LEVA_SLOZENA_ZAVORKA:
        {
            if (c == EOF)
            {
                stav = S_END_OF_FILE;
                //vrat_se_o_znak((char) c);
            }
            else if (c != '}')
            {
                stav = S_LEVA_SLOZENA_ZAVORKA;
            }
            else // mame aj pravu zlozenu = komentar
            {
                stav = S_START;
                i = 0;
                inicializuj_token();
                //stav = S_KOMENTAR;
            }
            break;
        }

        case S_PRAVA_SLOZENA_ZAVORKA:
        {
            stav = S_CHYBA;
            break;
        }
        /******************************************************************************************************************/
        case S_RETEZEC:
        {
			
          // pozor mozno tu tento IF nebude  >> bud '#65'  alebo ''#65''
          /*  if (token.data == NULL && c == '#') // jedna sa o samostatnu ESCAPE SEKV
            {
                    stav = S_ESCAPE_SEKVENCE;
                    // inicializujeme ESC zalezitosti !
                    ESCdata=NULL;
                    ESCi=0;
                    break;
            }
            */


            if (c != '\'')
            {
                if (c == EOF)
                {
                    stav = S_END_OF_FILE;
                    // nejaka chybycka !                                treba VYRIESIT TUTO CHYBU NEJAK !!!!!
                    break;
                }
                else if ((c == '\t') || (c == '\v')) /* soucasti retezce nesmi byt tabulator */
                {
				        napln_token(stav);
                        stav = S_CHYBA;
                        vrat_se_o_znak((char) c);
                        break;
				}

                stav = S_RETEZEC;
                vloz_znak_do_tokenu(c, &i);
            }

            else   // teraz som dostal uvodzovku '   c = getc(soubor);     ungetc(znak, soubor);
            {
                c = getc(soubor); // musime sa pozriet co je dalsie .. moze byt # alebo dalsi ' alebo nieco ine

                if (c == '#') // mame ESCAPE SEKVENCIU !
                {
                    stav = S_ESCAPE_SEKVENCE;
                    // inicializujeme ESC zalezitosti !
                    ESCdata=NULL;
                    ESCi=0;

                }
                else if (c == '\'') // mame dve za sebu idece ''  >> to je v retazci jeden '  isdigit(c)
                {
				        napln_token(stav);
                        stav = S_CHYBA;
                        vrat_se_o_znak((char) c);
                }
                else
                {
				
                    if (  token.data == NULL )
                    {  
                        //c = '*';
                        //vloz_znak_do_tokenu(c, &i);
                        token.data = mymalloc(sizeof(char)+1);
                        token.data = "";
                       // printf(">>%s<<\n", token.data );
                    }
                    vrat_se_o_znak((char) c); // musime sa vratit  o znak, HURA MAME RETAZEC ULOZIME
                    napln_token(stav);
                    stav = S_END;
                }

            }
            break;
        }



        case S_ESCAPE_SEKVENCE: /* asci hodnota 0 - 255 POZOR NEosetruji*/
        {
            if (isdigit(c))
            {
                stav = S_ESCAPE_SEKVENCE;
                if ((ESCdata = (char *) myrealloc(ESCdata, (ESCi) + 2)))
                {
                    ESCdata[(ESCi) + 1] = '\0';
                    ESCdata[(ESCi)] = c;   // ulozime cislo
                    (ESCi)++;
                }
                else {
					error = interni_chyba_interpretu; /* interni chyba prekladace */
					fprintf(stderr, "CHYBA ALOKACE : %d \n",error );
					trashDestroy(interni_chyba_interpretu);
					
				}

            }
            else if (c == '-') /* asci hodnota musi byt kladna  */
            {
				        napln_token(stav);
                        stav = S_CHYBA;
                        vrat_se_o_znak((char) c);
			}
            else // uz nemame cislo ...
            {
                if (c == '\'') // ok ukoncili sme escape sekvenciu apostrofom vsetko ok :)
                {
                    /* prevedieme string na cislo a ulozime do tokenu hotovo */
                    int ASCIcislo = atoi(ESCdata);
                    myfree(ESCdata); // dealokujeme pamet pre escape sekvenciu
                    if (ASCIcislo > 0 && ASCIcislo < 256)
                    {
                        vloz_znak_do_tokenu(ASCIcislo, &i);
                        stav = S_RETEZEC;
                    }
                    else
                    {
                        // chyba cislo je mimo ASCI !!!
                        napln_token(stav);
                        stav = S_CHYBA;
                        vrat_se_o_znak((char) c);
                    }

                }
            }
            break;
        }

        /**********************************************************************************************************************/

        case S_KLIC_BEGIN:      // klucove slova
        case S_KLIC_BOOLEAN:
        case S_KLIC_DO:
        case S_KLIC_ELSE:
        case S_KLIC_END:
        case S_KLIC_FALSE:
        case S_KLIC_FORWARD:
        case S_KLIC_FUNCTION:
        case S_KLIC_IF:
        case S_KLIC_INTEGER:
        case S_KLIC_READLN:
        case S_KLIC_REAL:
        case S_KLIC_STRING:
        case S_KLIC_THEN:
        case S_KLIC_TRUE:
        case S_KLIC_VAR:
        case S_KLIC_WHILE:
        case S_KLIC_WRITE:      // klucove slova potialto

        case S_CARKA:
        case S_TECKA:
        case S_STREDNIK:
        case S_PRIRAZENI:
        case S_PLUS:
        case S_MINUS:
        case S_KRAT:
        case S_DELENO:
        case S_ROVNO:
        case S_LEVA_ZAVORKA:
        case S_PRAVA_ZAVORKA:
        case S_VETSI_NEBO_ROVNO:
        case S_MENSI_NEBO_ROVNO:
        case S_NEROVNO:
        case S_END_OF_FILE:
        {
            napln_token(stav);
            stav = S_END;
            vrat_se_o_znak((char) c);
            break;
        }

        case S_CHYBA:
        {
            //vloz_znak_do_tokenu(c, &i);
            napln_token(stav);
            error = chyba_v_programu_v_ramci_lexikalni_analyzy;
            //vrat_se_o_znak((char) c);
            konec = true;
            token.radek = radek;
            token.sloupec = sloupec;
            fprintf(stderr, "%d : LEXIKALNI CHYBA na souradnici [%d, %d] \n", error, radek+1, sloupec);
            trashDestroy(chyba_v_programu_v_ramci_lexikalni_analyzy); /* uklizim */
            break;
        }
        case S_END:
        {
            vrat_se_o_znak((char) c);
            konec = true;
            break;
        }
        default:
        {
            if (error) break;
//            if (c == '\n')
  //          {
    //                radek++;
      //              sloupec = 1;
        //    }
          //  else if (isprint(c))
            //    sloupec++;
        }
        }
                            if (c == '\n')
            {
                    radek++;
                    sloupec = 0;
            }
            else if (isprint(c))
                sloupec++;
    }
    return token;
}
