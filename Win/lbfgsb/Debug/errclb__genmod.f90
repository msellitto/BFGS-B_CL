        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:47:50 2012
        MODULE ERRCLB__genmod
          INTERFACE 
            SUBROUTINE ERRCLB(N,M,FACTR,L,U,NBD,TASK,INFO,K)
              INTEGER(KIND=4) :: N
              INTEGER(KIND=4) :: M
              REAL(KIND=8) :: FACTR
              REAL(KIND=8) :: L(N)
              REAL(KIND=8) :: U(N)
              INTEGER(KIND=4) :: NBD(N)
              CHARACTER(LEN=60) :: TASK
              INTEGER(KIND=4) :: INFO
              INTEGER(KIND=4) :: K
            END SUBROUTINE ERRCLB
          END INTERFACE 
        END MODULE ERRCLB__genmod
