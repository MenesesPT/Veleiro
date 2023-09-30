/*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*                                                      */
/* File:  Level1.h                                      */
/*                                                      */
/* Author: Automatically generated by Xfuzzy            */
/*                                                      */
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef XFUZZY
#define XFUZZY
/*------------------------------------------------------*/
/* Fuzzy Number                                         */
/*------------------------------------------------------*/

 typedef struct {
   double (* equal)(double x);
   double (* center)();
   double (* basis)();
   double (* param)(int i);
 } Consequent;

 typedef struct {
   double min;
   double max;
   double step;
   double (* imp)(double a, double b);
   double (* also)(double a, double b);
   int length;
   double* degree;
   int inputlength;
   double* input;
   Consequent* conc;
 } FuzzyNumber;

#endif /* XFUZZY */


#ifndef XFUZZY_Level1
#define XFUZZY_Level1
/*------------------------------------------------------*/
/* Inference Engine                                     */
/*------------------------------------------------------*/

void Level1InferenceEngine(double angulo_correcao, double Cata_vento, double Roll, double* Leme, double* Vela);

#endif /* XFUZZY_Level1 */
