/**
 * Define a grammar called Hello
 */
grammar MOS6502;

options             { language=Cpp; }
r                   : line+ end_directive? EOF;         // non empty sequence of assembler instructions

line                : label? (directive | statement);

directive           : byte_directive | word_directive | dbyte_directive | org_directive | ass_directive;
statement           : dir_statement | imm_statement | rel_statement | idx_statement | idr_statement | idx_idr_statement | idr_idx_statement;

dir_statement       : dir_opcode;
dir_opcode          : ('BRK' | 'PHP' | 'ASL' | 'CLC' | 'PLP' | 'ROL' | 'SEC' | 'RTI' | 'PHA' | 'LSR' | 'CLI' | 'RTS' | 'PLA' | 'ROR' | 'SEI' 
                    |  'DEY' | 'TXA' | 'TYA' | 'TXS' | 'TAY' | 'TAX' | 'CLV' | 'TSX' | 'INY' | 'DEX' | 'CLD' | 'INX' | 'NOP' | 'SED');

imm_statement       : imm_opcode imm_operand;
imm_opcode          : ('ORA' | 'AND' | 'EOR' | 'ADC' | 'LDY' | 'LDX' | 'LDA'  | 'CPY' | 'CMP' | 'CPX' | 'SBC');
imm_operand         : POUND (numerical_byte | expression);

rel_statement       : rel_opcode symbol;
rel_opcode          : ('BPL' | 'BMI' | 'BVC' | 'BVS' | 'BCC' | 'BCS' | 'BNE' | 'BEQ' );

idx_statement       : idx_x_statement //(idx_opcode idxy_operand idx_x) 
                    | idx_y_statement // (idy_opcode idxy_operand idx_y)
                    | idx_abs_statement; // (id_opcode idxy_operand);
                    
idx_x_statement     : idx_opcode idxy_operand idx_x;
idx_y_statement     : idy_opcode idxy_operand idx_y;
idx_abs_statement   : idabs_opcode idxy_operand;
                    
                    
idx_opcode          : ('ORA' | 'ASL' | 'AND' | 'ROL' | 'EOR' | 'LSR' | 'ADC' | 'ROR' | 'STY' | 'STA' | 'LDY' | 'LDA' | 'CMP' | 'DEC' | 'SBC' | 'INC' );
idy_opcode          : ('ORA' | 'AND' | 'EOR' | 'ADC' | 'STX' | 'STA' | 'LDX' | 'LDA' | 'CMP' | 'SBC' );
idabs_opcode        : ('ORA' | 'ASL' | 'JSR' | 'BIT' | 'AND' | 'ROL' | 'JMP' | 'EOR' | 'LSR' | 'ADC' | 'ROR' | 'STY' | 'STA' | 'STX' | 'LDY' | 'LDA' 
                    |  'LDX' | 'CPY' | 'CMP' | 'DEC' | 'CPX' | 'SBC' | 'INC' );
idxy_operand        : (dec8 | dec | char8 | bin8 | hex8 | hex16 | expression);
idx_x               : IDX_X;
idx_y               : IDX_Y;

idr_statement       : idr_opcode LBRACE idr_operand RBRACE;
idr_opcode          : 'JMP';
idr_operand         : (dec8 | dec | char8 | bin8 | hex8 | hex16 | expression);


idx_idr_statement   : idx_idr_idx_opcode LBRAKET idx_idr_idx_operand IDX_X RBRAKET;
idr_idx_statement   : idx_idr_idx_opcode LBRAKET idx_idr_idx_operand RBRAKET IDX_Y;
idx_idr_idx_opcode  : ('ORA' | 'AND' | 'EOR' | 'ADC' | 'STA' | 'LDA' | 'CMP' | 'SBC');
idx_idr_idx_operand : (dec8 | dec | char8 | bin8 | hex8 | hex16 | expression) ;


dec8                : DEC8;
char8               : CHAR8;
bin8                : BIN8;
hex8                : HEX8;
hex16               : HEX16;
dec                 : DEC;


expression          : expression (DIV | MUL | PERCENT) expression
                    | expression (SUB | ADD) expression
                    | symbol
                    | numerical 
                    | LBRACE expression RBRACE;
                    
label               : ID COLON;
symbol              : ID;

numerical_byte      : dec8 | char8 | bin8 | hex8;
numerical           : numerical_byte | dec | hex16;

byte_directive      : DOT 'BYTE' data_list;
word_directive      : DOT 'WORD' data_list; // turns two byte entities into LE
dbyte_directive     : DOT 'DBYTE' data_list; // leaves data bytes in given order
end_directive       : DOT 'END'; // indicates end of source file
org_directive       : DOT 'ORG' expression;
ass_directive       : ID EQUALS expression;


data_list           : data (COMMA data)*;
data                : (expression | data_string); // anything that can be put into memory
data_string         : STRING;

fragment STRING_CH  : [\u0000-\u0021\u0023-\u00ff]; // anything in ascii except QUOTE

DEC8                : '0' | [1-9][0-9]? | '1'[0-9][0-9] | '2'[0-4][0-9] | '25'[0-5];
CHAR8               : APO [\u0000-\u0026\u0028-\u00ff] APO; // anything in ascii except APO
STRING              : QUOTE STRING_CH STRING_CH+ QUOTE;
BIN8                : PERCENT ([01] | [01][01] | [01][01][01] | [01][01][01][01] | [01][01][01][01][01] 
                    | [01][01][01][01][01][01] | [01][01][01][01][01][01][01] 
                    | [01][01][01][01][01][01][01][01]);
HEX8                : DOLLAR ([0-9a-fA-F] | [0-9a-fA-F][0-9a-fA-F]);
HEX16               : DOLLAR ([0-9a-fA-F][0-9a-fA-F][0-9a-fA-F] | [0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F]);
DEC                 : [1-9][0-9]*; // '0' is not included, since that is is covered in DEC8

IDX_X               : COMMA ('x'|'X');
IDX_Y               : COMMA ('y'|'Y');

POUND               : '#';
COMMA               : ',';
DOT                 : '.';
APO                 : '\'';
QUOTE               : '"';
DOLLAR              : '$';
PERCENT             : '%';
LBRACE              : '(';
RBRACE              : ')';
LBRAKET             : '[';
RBRAKET             : ']';
ADD                 : '+';
SUB                 : '-';
DIV                 : '/';
MUL                 : '*';
EQUALS              : '=';
COLON               : ':';
ID                  : [_a-zA-Z][_a-zA-Z0-9]* ;             // C style identifier

// skip everything between semicolon and newline, skip any other kind of WS
COMMENT             : ';' .*? '\n' -> skip;
WS                  : [ \t\r\n]+ -> skip ;
