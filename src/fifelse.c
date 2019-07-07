#include <R.h>
#include <Rdefines.h>
#include <R_ext/Error.h>
#include<stdint.h>

SEXP fifelseR(SEXP l, SEXP a, SEXP b)
{
  // l is the test
  // a is what to do in case l is TRUE
  // b is what to do in case l is FALSE
  // maybe add something in case l is neither TRUE or FALSE ? for now it will default to NA
  
  // Check if test is logical
  if(!isLogical(l)) error("Argument 'test' must be logical.");
  
  const uint64_t len0 = LENGTH(l);
  const uint64_t len1 = LENGTH(a);
  const uint64_t len2 = LENGTH(b);
  
  SEXPTYPE ta = TYPEOF(a);
  SEXPTYPE tb = TYPEOF(b);
  
  unsigned int stack_size = 0;
  
  // Check if same type and do en-listing of singleton
  if(ta != tb)
  {
    if(ta == VECSXP && (tb == INTSXP || tb == REALSXP || tb == LGLSXP || tb == CPLXSXP))
    {
      if(len2 == 1)
      {
        SEXP tmp = PROTECT(duplicate(b)); stack_size++;
        b = PROTECT(allocVector(VECSXP,1)); stack_size++;
        SET_VECTOR_ELT(b, 0, tmp);
        tb = TYPEOF(b);
      } 
    } else if(tb == VECSXP && (ta == INTSXP || ta == REALSXP || ta == LGLSXP || ta == CPLXSXP)){
      if(len1 == 1)
      {
        SEXP tmp = PROTECT(duplicate(a));stack_size++;
        a = PROTECT(allocVector(VECSXP,1)); stack_size++;
        SET_VECTOR_ELT(a, 0, tmp);
        ta = TYPEOF(a);
      }
    } else {
      error("Item 'yes' is of type %s but item 'no' is of type %s. Please make sure they are of the same type.", type2char(ta),type2char(tb));
    }
  }
    
  SEXP class_type = PROTECT(getAttrib(a,R_ClassSymbol)); stack_size++;
  
  // Check if same class
  if(!R_compute_identical(class_type,getAttrib(b,R_ClassSymbol),0))
    error("Item 'yes' has different class than item 'no'. Please make sure that they have same class type.");
  
  // Check if factor
  if(isFactor(a))
  {
    a = PROTECT(asCharacterFactor(a));
    b = PROTECT(asCharacterFactor(b));
    stack_size = stack_size + 2;
    ta = STRSXP;
  }
  //Jan Gorecki : Afair this will make factor class always slower than character, would be nice to have it optimised where possible
  
  /*
   Variable 'adj' is used to seperate case where l (test),
   a (yes) and b (no) are of different length.
   */
  unsigned int adj = 0;
  
  // Check here the length of the different input variables.
  if(len1 != len2) 
  {
    if(len1 < len2)
    {
      if(len1 != 1)    error("Length of 'yes' must be 1 or equal to length of 'test'.");
      if(len2 != len0) error("Length of 'no' must be 1 or equal to length of 'test'.");
      adj = 1;
    } else {
      if(len2 != 1)    error("Length of 'no' must be 1 or equal to length of 'test'.");
      if(len1 != len0) error("Length of 'yes' must be 1 or equal to length of 'test'.");
      adj = 2;
    }
  } else {
    if(len0 != len1)
    {
      if(len1 != 1) error("Length of 'yes' and 'no' must be 1 or equal to length of 'test'.");
      adj = 3;
    }
  }
  
  SEXP result = R_NilValue;
  int *pl = LOGICAL(l);
  uint64_t i;
  
  switch(ta)
  {
    /*
     This part is for dealing with a (yes) and b (no)
     in case they are both of type INTSXP (integer)
     */
  case INTSXP :
    result = PROTECT(allocVector(INTSXP, len0));
    stack_size++;
    int *int_pres, *int_pa, *int_pb;
    int_pres = INTEGER(result);
    int_pa   = INTEGER(a);
    int_pb   = INTEGER(b);
    switch(adj)
    {
    case 0:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : int_pres[i] = int_pb[i];  break;
        case 1  : int_pres[i] = int_pa[i];  break;
        default : int_pres[i] = NA_INTEGER; break;
        }
      }
      break;
      
    case 1:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : int_pres[i] = int_pb[i];  break;
        case 1  : int_pres[i] = int_pa[0];  break;
        default : int_pres[i] = NA_INTEGER; break;
        }
      }
      break;
      
    case 2:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : int_pres[i] = int_pb[0];  break;
        case 1  : int_pres[i] = int_pa[i];  break;
        default : int_pres[i] = NA_INTEGER; break;
        }
      }
      break;
      
    case 3:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : int_pres[i] = int_pb[0];  break;
        case 1  : int_pres[i] = int_pa[0];  break;
        default : int_pres[i] = NA_INTEGER; break;
        }
      }
      break;
    }
    break;
    
    /*
     This part is for dealing with a (yes) and b (no)
     in case they are both of type REALSXP (double)
     */
  case REALSXP :
    result = PROTECT(allocVector(REALSXP, len0));
    stack_size++;
    double *double_pres, *double_pa, *double_pb;
    double_pres = REAL(result);
    double_pa   = REAL(a);
    double_pb   = REAL(b);
    switch(adj)
    {
    case 0:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : double_pres[i] = double_pb[i];  break;
        case 1  : double_pres[i] = double_pa[i];  break;
        default : double_pres[i] = NA_REAL; break;
        }
      }
      break;
      
    case 1:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : double_pres[i] = double_pb[i];  break;
        case 1  : double_pres[i] = double_pa[0];  break;
        default : double_pres[i] = NA_REAL; break;
        }
      }
      break;
      
    case 2:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : double_pres[i] = double_pb[0];  break;
        case 1  : double_pres[i] = double_pa[i];  break;
        default : double_pres[i] = NA_REAL; break;
        }
      }
      break;
      
    case 3:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : double_pres[i] = double_pb[0];  break;
        case 1  : double_pres[i] = double_pa[0];  break;
        default : double_pres[i] = NA_REAL; break;
        }
      }
      break;
    }
    break;
    
    /*
     This part is for dealing with a (yes) and b (no)
     in case they are both of type LGLSXP (logical)
     */
  case LGLSXP :
    result = PROTECT(allocVector(LGLSXP, len0));
    stack_size++;
    int *lg_pres, *lg_pa, *lg_pb;
    lg_pres = LOGICAL(result);
    lg_pa   = LOGICAL(a);
    lg_pb   = LOGICAL(b);
    switch(adj)
    {
    case 0:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : lg_pres[i] = lg_pb[i];  break;
        case 1  : lg_pres[i] = lg_pa[i];  break;
        default : lg_pres[i] = NA_LOGICAL; break;
        }
      }
      break;
      
    case 1:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : lg_pres[i] = lg_pb[i];  break;
        case 1  : lg_pres[i] = lg_pa[0];  break;
        default : lg_pres[i] = NA_LOGICAL; break;
        }
      }
      break;
      
    case 2:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : lg_pres[i] = lg_pb[0];  break;
        case 1  : lg_pres[i] = lg_pa[i];  break;
        default : lg_pres[i] = NA_LOGICAL; break;
        }
      }
      break;
      
    case 3:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : lg_pres[i] = lg_pb[0];  break;
        case 1  : lg_pres[i] = lg_pa[0];  break;
        default : lg_pres[i] = NA_LOGICAL; break;
        }
      }
      break;
    }
    break;
    
    /*
     This part is for dealing with a (yes) and b (no)
     in case they are both of type STRSXP (character)
     */
  case STRSXP :
    result = PROTECT(allocVector(STRSXP, len0));
    stack_size++;
    switch(adj)
    {
    case 0:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : SET_STRING_ELT(result, i, STRING_ELT(b, i));  break;
        case 1  : SET_STRING_ELT(result, i, STRING_ELT(a, i));  break;
        default : SET_STRING_ELT(result, i, NA_STRING); break;
        }
      }
      break;
      
    case 1:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : SET_STRING_ELT(result, i, STRING_ELT(b, i));  break;
        case 1  : SET_STRING_ELT(result, i, STRING_ELT(a, 0));  break;//some speed up can be done here
        default : SET_STRING_ELT(result, i, NA_STRING); break;
        }
      }
      break;
      
    case 2:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : SET_STRING_ELT(result, i, STRING_ELT(b, 0));  break;
        case 1  : SET_STRING_ELT(result, i, STRING_ELT(a, i));  break;
        default : SET_STRING_ELT(result, i, NA_STRING); break;
        }
      }
      break;
      
    case 3:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : SET_STRING_ELT(result, i, STRING_ELT(b, 0));  break;
        case 1  : SET_STRING_ELT(result, i, STRING_ELT(a, 0));  break;
        default : SET_STRING_ELT(result, i, NA_STRING); break;
        }
      }
      break;
    }
    break;
    
    /*
     This part is for dealing with a (yes) and b (no)
     in case they are both of type CPLXSXP (complex)
     */
  case CPLXSXP :
    result = PROTECT(allocVector(CPLXSXP, len0));
    Rcomplex *cp_pres, *cp_pa, *cp_pb;
    Rcomplex NA_CPLX = { NA_REAL, NA_REAL }; // taken from subset.c
    cp_pres = COMPLEX(result);
    cp_pa   = COMPLEX(a);
    cp_pb   = COMPLEX(b);
    stack_size++;
    switch(adj)
    {
    case 0:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : cp_pres[i] = cp_pb[i]; break;
        case 1  : cp_pres[i] = cp_pa[i]; break;
        default : cp_pres[i] = NA_CPLX;  break;
        }
      }
      break;
      
    case 1:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : cp_pres[i] = cp_pb[i]; break;
        case 1  : cp_pres[i] = cp_pa[0]; break;
        default : cp_pres[i] = NA_CPLX;  break;
        }
      }
      break;
      
    case 2:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : cp_pres[i] = cp_pb[0]; break;
        case 1  : cp_pres[i] = cp_pa[i]; break;
        default : cp_pres[i] = NA_CPLX;  break;
        }
      }
      break;
      
    case 3:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : cp_pres[i] = cp_pb[0]; break;
        case 1  : cp_pres[i] = cp_pa[0]; break;
        default : cp_pres[i] = NA_CPLX;  break;
        }
      }
      break;
    }
    break;
    
    /*
     This part is for dealing with a (yes) and b (no)
     in case they are both of type VECSXP (list)
     */
  case VECSXP :
    result = PROTECT(allocVector(VECSXP, len0)); stack_size++;
    SEXP nms = PROTECT(allocVector(STRSXP, len0)); stack_size++; // create list names
    // Get list names from a and b
    SEXP nmsa = PROTECT(getAttrib(a,R_NamesSymbol)); stack_size++;
    SEXP nmsb = PROTECT(getAttrib(b,R_NamesSymbol)); stack_size++;
    SEXP na_list = PROTECT(allocVector(INTSXP, 1)); stack_size++;
    INTEGER(na_list)[0] = NA_INTEGER;
    switch(adj)
    {
    case 0:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : SET_VECTOR_ELT(result, i, VECTOR_ELT(b, i)); if(!isNull(nmsb))SET_STRING_ELT(nms, i, STRING_ELT(nmsb,i)); break;
        case 1  : SET_VECTOR_ELT(result, i, VECTOR_ELT(a, i)); if(!isNull(nmsa))SET_STRING_ELT(nms, i, STRING_ELT(nmsa,i)); break;
        default : SET_VECTOR_ELT(result, i, na_list); break;
        }
      }
      break;
      
    case 1:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : SET_VECTOR_ELT(result, i, VECTOR_ELT(b, i)); if(!isNull(nmsb))SET_STRING_ELT(nms, i, STRING_ELT(nmsb,i)); break;
        case 1  : SET_VECTOR_ELT(result, i, VECTOR_ELT(a, 0)); if(!isNull(nmsa))SET_STRING_ELT(nms, i, STRING_ELT(nmsa,0)); break;
        default : SET_VECTOR_ELT(result, i, na_list); break;
        }
      }
      break;
      
    case 2:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : SET_VECTOR_ELT(result, i, VECTOR_ELT(b, 0)); if(!isNull(nmsb))SET_STRING_ELT(nms, i, STRING_ELT(nmsb,0)); break; //some speed up can be done here
        case 1  : SET_VECTOR_ELT(result, i, VECTOR_ELT(a, i)); if(!isNull(nmsa))SET_STRING_ELT(nms, i, STRING_ELT(nmsa,i)); break;break;
        default : SET_VECTOR_ELT(result, i, na_list); break;
        }
      }
      break;
      
    case 3:
      for(i = 0; i < len0; i++)
      {
        switch(pl[i])
        {
        case 0  : SET_VECTOR_ELT(result, i, VECTOR_ELT(b, 0)); if(!isNull(nmsb))SET_STRING_ELT(nms, i, STRING_ELT(nmsb,0)); break;
        case 1  : SET_VECTOR_ELT(result, i, VECTOR_ELT(a, 0)); if(!isNull(nmsa))SET_STRING_ELT(nms, i, STRING_ELT(nmsa,0)); break;
        default : SET_VECTOR_ELT(result, i, na_list); break;
        }
      }
      break;
    }
    if(!isNull(nmsa) || !isNull(nmsb)) setAttrib(result, R_NamesSymbol, nms);
    break;
    
  default: error("Type %s is not supported.",type2char(ta)); break;
  }
  
  // Check class type and adjust (except for factor)
  if(!isNull(class_type) && !isFactor(a)) copyMostAttrib(a, result); // issue here for factor with NA value - moved to R code
    
  UNPROTECT(stack_size);
  return result;
}
