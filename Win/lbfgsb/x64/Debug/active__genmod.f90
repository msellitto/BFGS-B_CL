        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:56:58 2012
        MODULE ACTIVE__genmod
          INTERFACE 
            SUBROUTINE ACTIVE(N,L,U,NBD,X,IWHERE,IPRINT,PRJCTD,CNSTND,  &
     &BOXED)
              INTEGER(KIND=4) :: N
              REAL(KIND=8) :: L(N)
              REAL(KIND=8) :: U(N)
              INTEGER(KIND=4) :: NBD(N)
              REAL(KIND=8) :: X(N)
              INTEGER(KIND=4) :: IWHERE(N)
              INTEGER(KIND=4) :: IPRINT
              LOGICAL(KIND=4) :: PRJCTD
              LOGICAL(KIND=4) :: CNSTND
              LOGICAL(KIND=4) :: BOXED
            END SUBROUTINE ACTIVE
          END INTERFACE 
        END MODULE ACTIVE__genmod
