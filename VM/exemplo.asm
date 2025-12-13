; =================================================================
; Teste de Definição de Macro Aninhada (MACRO dentro de MACRO)
; =================================================================

; Macro externa que contém definição de macro interna
OUTER MACRO &P1, &P2
    ; Macro interna definida dentro da externa
    INNER MACRO &Q
        LOAD &Q
        MUL #2
        STORE &Q
    MEND
    ; Código da macro externa
    LOAD &P1
    ADD &P2
    STORE &P1
MEND

; Outra macro aninhada
COMPLEX MACRO &A, &B
    HELPER MACRO &X
        LOAD &X
        ADD #1
        STORE &X
    MEND
    LOAD &A
    ADD &B
    STORE &A
MEND

; Programa
START:  OUTER VAR1, VAR2
        COMPLEX NUM1, NUM2
        STOP

; Dados
VAR1:   CONST 5
VAR2:   CONST 10
NUM1:   CONST 15
NUM2:   CONST 20