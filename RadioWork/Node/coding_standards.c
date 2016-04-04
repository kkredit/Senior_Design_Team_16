/* 
 * coding_standards.c
 * 
 * A file to centrally define the coding standards to be used for all code that is part of Team 16's
 * senior design project. 
 * 
 * (C) 2016, John Connell, Anthony Jin, Charles Kingston, and Kevin Kredit
 * Last Modified: 4/1/16
 */
 
 
 /* General Comments */
 
 // If possible, keep lines limited to 100 characters wide. This line is exactly 100 characters.....
 
 // There should be sufficient commentary inside the code so that someone who hasn't seen it before
 // can have at least the general idea of what is going on.
 
 // Indents should be two spaces, no tabs
 
 // Files should end with exactly one empty line
 
 
 /* Names */
 
 // -- Variables --
 // "#defines" and public constants shall be ALL_CAPS
 // else should be camelCoded
 
 // -- Functions --
 // camelCoded
 
 // -- Classes --
 // CamelCoded (first letter is capital)
 
 
 /* Headers */
 
 // -- Beginning of file --
/* 
 * [filename.filetype]
 * 
 * Descriptive sentence explaining what the file contains, its purpose, and its role in the overall
 * system. Split accross multiple lines if necessary.
 * 
 * (C) 2016, John Connell, Anthony Jin, Charles Kingston, and Kevin Kredit
 * Last Modified: [datelastmodified]
 */

// -- Functions --
/* 
 * functionName()
 *
 * Descriptive sentence saying what the function does.
 * 
 * @preconditions: as applicable
 * @postconditions: as applicable
 * 
 * @param type varname: describe what it should be and its purpose (if applicable)
 * ^^ repeat for each parameter^^
 * 
 * @return type varname: describe the return variable (if applicable)
 */ 
 