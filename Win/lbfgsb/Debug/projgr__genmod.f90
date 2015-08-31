        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:47:50 2012
        MODULE PROJGR__genmod
          INTERFACE 
            SUBROUTINE PROJGR(N,L,U,NBD,X,G,SBGNRM)
              INTEGER(KIND=4) :: N
              REAL(KIND=8) :: L(N)
              REAL(KIND=8) :: U(N)
              INTEGER(KIND=4) :: NBD(N)
              REAL(KIND=8) :: X(N)
              REAL(KIND=8) :: G(N)
              REAL(KIND=8) :: SBGNRM
            END SUBROUTINE PROJGR
          END INTERFACE 
        END MODULE PROJGR__genmod
