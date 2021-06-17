#include "data.table.h"

void nafillDouble(double *x, uint_fast64_t nx, unsigned int type, double fill, bool nan_is_na, ans_t *ans, bool verbose) {
  double tic=0.0;
  if (verbose)
    tic = omp_get_wtime();
  if (type==0) { // const
    if (nan_is_na) {
      for (uint_fast64_t i=0; i<nx; i++) {
        ans->dbl_v[i] = ISNAN(x[i]) ? fill : x[i];
      }
    } else {
      for (uint_fast64_t i=0; i<nx; i++) {
        ans->dbl_v[i] = ISNA(x[i]) ? fill : x[i];
      }
    }
  } else if (type==1) { // locf
    if (nan_is_na) {
      ans->dbl_v[0] = ISNAN(x[0]) ? fill : x[0];
      for (uint_fast64_t i=1; i<nx; i++) {
        ans->dbl_v[i] = ISNAN(x[i]) ? ans->dbl_v[i-1] : x[i];
      }
    } else {
      ans->dbl_v[0] = ISNA(x[0]) ? fill : x[0];
      for (uint_fast64_t i=1; i<nx; i++) {
        ans->dbl_v[i] = ISNA(x[i]) ? ans->dbl_v[i-1] : x[i];
      }
    }
  } else if (type==2) { // nocb
    if (nan_is_na) {
      ans->dbl_v[nx-1] = ISNAN(x[nx-1]) ? fill : x[nx-1];
      for (int_fast64_t i=nx-2; i>=0; i--) {
        ans->dbl_v[i] = ISNAN(x[i]) ? ans->dbl_v[i+1] : x[i];
      }
    } else {
      ans->dbl_v[nx-1] = ISNA(x[nx-1]) ? fill : x[nx-1];
      for (int_fast64_t i=nx-2; i>=0; i--) {
        ans->dbl_v[i] = ISNA(x[i]) ? ans->dbl_v[i+1] : x[i];
      }
    }
  }
  if (verbose)
    snprintf(ans->message[0], 500, "%s: took %.3fs\n", __func__, omp_get_wtime()-tic);
}
void nafillInteger(int32_t *x, uint_fast64_t nx, unsigned int type, int32_t fill, ans_t *ans, bool verbose) {
  double tic=0.0;
  if (verbose)
    tic = omp_get_wtime();
  if (type==0) { // const
    //Rprintf("fill=%d\n",fill);
    for (uint_fast64_t i=0; i<nx; i++) {
      //Rprintf("before write: x[%d]=%d\n",i,x[i]);
      ans->int_v[i] = x[i]==NA_INTEGER ? fill : x[i];
      //Rprintf("after write: ans->int_v[%d]=%d\n",i,ans->int_v[i]);
    }
    //Rprintf("ans->int_v[nx-1]=%d\n",ans->int_v[nx-1]);
  } else if (type==1) { // locf
    ans->int_v[0] = x[0]==NA_INTEGER ? fill : x[0];
    for (uint_fast64_t i=1; i<nx; i++) {
      ans->int_v[i] = x[i]==NA_INTEGER ? ans->int_v[i-1] : x[i];
    }
  } else if (type==2) { // nocb
    ans->int_v[nx-1] = x[nx-1]==NA_INTEGER ? fill : x[nx-1];
    for (int_fast64_t i=nx-2; i>=0; i--) {
      ans->int_v[i] = x[i]==NA_INTEGER ? ans->int_v[i+1] : x[i];
    }
  }
  if (verbose)
    snprintf(ans->message[0], 500, "%s: took %.3fs\n", __func__, omp_get_wtime()-tic);
}
void nafillInteger64(int64_t *x, uint_fast64_t nx, unsigned int type, int64_t fill, ans_t *ans, bool verbose) {
  double tic=0.0;
  if (verbose)
    tic = omp_get_wtime();
  if (type==0) { // const
    for (uint_fast64_t i=0; i<nx; i++) {
      ans->int64_v[i] = x[i]==NA_INTEGER64 ? fill : x[i];
    }
  } else if (type==1) { // locf
    ans->int64_v[0] = x[0]==NA_INTEGER64 ? fill : x[0];
    for (uint_fast64_t i=1; i<nx; i++) {
      ans->int64_v[i] = x[i]==NA_INTEGER64 ? ans->int64_v[i-1] : x[i];
    }
  } else if (type==2) { // nocb
    ans->int64_v[nx-1] = x[nx-1]==NA_INTEGER64 ? fill : x[nx-1];
    for (int_fast64_t i=nx-2; i>=0; i--) {
      ans->int64_v[i] = x[i]==NA_INTEGER64 ? ans->int64_v[i+1] : x[i];
    }
  }
  if (verbose)
    snprintf(ans->message[0], 500, "%s: took %.3fs\n", __func__, omp_get_wtime()-tic);
}
void nafillString(const SEXP *x, uint_fast64_t nx, unsigned int type, SEXP fill, ans_t *ans, bool verbose) {
  double tic=0.0;
  if (verbose)
    tic = omp_get_wtime();
  if (type==0) { // const
    //for (uint_fast64_t i=0; i<nx; i++) Rprintf("x[%d]=%s\n", i, CHAR(x[i]));
    for (uint_fast64_t i=0; i<nx; i++) {
      SET_STRING_ELT(ans->char_v, i, x[i]==NA_STRING ? fill : x[i]);
      //Rprintf("x[%d]=%s  ->  ", i, CHAR(x[i]));
      //SEXP tmp = x[i]==NA_STRING ? fill : x[i]; // setnafill handle to not update when reading
      //Rprintf("tmp=%s  ->  ", CHAR(tmp));
      //SET_STRING_ELT((SEXP)ans->char_v, i, tmp);
      //Rprintf("y[%d]=%s\n", i, CHAR(STRING_ELT((SEXP)ans->char_v, i)));
    }
    //for (uint_fast64_t i=0; i<nx; i++) Rprintf("y[%d]=%s\n", i, CHAR(STRING_ELT((SEXP)ans->char_v, i)));
  } else if (type==1) { // locf
    SET_STRING_ELT(ans->char_v, 0, x[0]==NA_STRING ? fill : x[0]);
    const SEXP* thisans = SEXPPTR_RO(ans->char_v); // takes out STRING_ELT from loop
    for (uint_fast64_t i=1; i<nx; i++) {
      SET_STRING_ELT(ans->char_v, i, x[i]==NA_STRING ? thisans[i-1] : x[i]);
    }
  } else if (type==2) { // nocb
    SET_STRING_ELT(ans->char_v, nx-1, x[nx-1]==NA_STRING ? fill : x[nx-1]);
    const SEXP* thisans = SEXPPTR_RO(ans->char_v); // takes out STRING_ELT from loop
    for (int_fast64_t i=nx-2; i>=0; i--) {
      SET_STRING_ELT(ans->char_v, i, x[i]==NA_STRING ? thisans[i+1] : x[i]);
    }
  }
  if (verbose)
    snprintf(ans->message[0], 500, "%s: took %.3fs\n", __func__, omp_get_wtime()-tic);
}

SEXP nafillR(SEXP obj, SEXP type, SEXP fill, SEXP nan_is_na_arg, SEXP inplace, SEXP cols) {
  int protecti=0;
  const bool verbose = GetVerbose();

  if (!xlength(obj))
    return(obj);

  double tic=0.0;
  if (verbose)
    tic = omp_get_wtime();

  bool binplace = LOGICAL(inplace)[0];
  if (!IS_TRUE_OR_FALSE(nan_is_na_arg))
    error("nan_is_na must be TRUE or FALSE"); // # nocov
  bool nan_is_na = LOGICAL(nan_is_na_arg)[0];

  SEXP x = R_NilValue;
  bool obj_scalar = isVectorAtomic(obj);
  if (obj_scalar) {
    if (binplace)
      error(_("'x' argument is atomic vector, in-place update is supported only for list/data.table"));
    else if (!isReal(obj) && !isInteger(obj) && !isLogical(obj) && !isFactor(obj) && !isString(obj))
      error(_("'x' argument must be numeric/integer/logical/factor/character/integer64, or list/data.table of such types"));
    SEXP obj1 = obj;
    obj = PROTECT(allocVector(VECSXP, 1)); protecti++; // wrap into list
    SET_VECTOR_ELT(obj, 0, obj1);
  }
  SEXP ricols = PROTECT(colnamesInt(obj, cols, ScalarLogical(TRUE))); protecti++; // nafill cols=NULL which turns into seq_along(obj)
  x = PROTECT(allocVector(VECSXP, length(ricols))); protecti++;
  int *icols = INTEGER(ricols);
  bool hadChar = false;
  bool* wasChar = (bool*)R_alloc(length(ricols), sizeof(bool)); // this is not yet used but can be used to run alll non-char columns in parallel region and char in single threaded
  for (int i=0; i<length(ricols); i++) {
    SEXP this_col = VECTOR_ELT(obj, icols[i]-1);
    if (isString(this_col)) {
      hadChar = true;
      wasChar[i] = true;
    } else {
      wasChar[i] = false;
      if (!isReal(this_col) && !isInteger(this_col) && !isLogical(this_col) && !isFactor(this_col))
        error(_("'x' argument must be numeric/integer/logical/factor/character/integer64, or list/data.table of such types"));
    }
    SET_VECTOR_ELT(x, i, this_col);
  }
  R_len_t nx = length(x);

  // data pointers
  double **dx = (double**)R_alloc(nx, sizeof(double*));
  int32_t **ix = (int32_t**)R_alloc(nx, sizeof(int32_t*)); // also logical and factor
  const SEXP **sx = (const SEXP**)R_alloc(nx, sizeof(SEXP*)); // character
  int64_t **i64x = (int64_t**)R_alloc(nx, sizeof(int64_t*));
  // nrows of columns
  uint_fast64_t *inx = (uint_fast64_t*)R_alloc(nx, sizeof(uint_fast64_t));
  ans_t *vans = (ans_t *)R_alloc(nx, sizeof(ans_t));
  for (R_len_t i=0; i<nx; i++) {
    const SEXP xi = VECTOR_ELT(x, i);
    inx[i] = xlength(xi);
    // not sure why these pointers are being constructed like this; TODO: simplify structure // A: because they are used in `ans_t` struct, strictly speaking, one of them, the expected one, is used in that struct.
    if (isReal(xi)) {
      dx[i] = REAL(xi);
      i64x[i] = (int64_t *)REAL(xi);
      ix[i] = NULL; sx[i] = NULL;
    } else if (isInteger(xi) || isLogical(xi) || isFactor(xi)) {
      ix[i] = INTEGER(xi);
      dx[i] = NULL; i64x[i] = NULL; sx[i] = NULL;
    } else if (isString(xi)) {
      ix[i] = NULL; dx[i] = NULL; i64x[i] = NULL;
      sx[i] = STRING_PTR(xi);
    } else {
      error(_("internal error: unknown column type, should have been caught by now, please report")); // # nocov
    }
  }
  SEXP ans = R_NilValue;
  if (!binplace) {
    ans = PROTECT(allocVector(VECSXP, nx)); protecti++;
    for (R_len_t i=0; i<nx; i++) {
      SET_VECTOR_ELT(ans, i, allocVector(TYPEOF(VECTOR_ELT(x, i)), inx[i]));
      const SEXP ansi = VECTOR_ELT(ans, i);
      const void *p =
        isReal(ansi) ? (void *)REAL(ansi) : (
          isInteger(ansi) ? (void *)INTEGER(ansi) : (
            isLogical(ansi) ? (void *)LOGICAL(ansi) : (void *)ansi
        )
      );
      vans[i] = ((ans_t) { .dbl_v=(double *)p, .int_v=(int *)p, .int64_v=(int64_t *)p, .char_v=(SEXP)p, .status=0, .message={"\0","\0","\0","\0"} });
    }
  } else {
    //ans = x; // only for debugging
    if (hadChar)
      error("setnafill and character column not yet implemented");
    for (R_len_t i=0; i<nx; i++) {
      // TODO character support, proper cast of .char_v below
      vans[i] = ((ans_t) { .dbl_v=dx[i], .int_v=ix[i], .int64_v=i64x[i], .char_v=(SEXP)sx[i], .status=0, .message={"\0","\0","\0","\0"} });
    }
  }

  unsigned int itype;
  if (!strcmp(CHAR(STRING_ELT(type, 0)), "const"))
    itype = 0;
  else if (!strcmp(CHAR(STRING_ELT(type, 0)), "locf"))
    itype = 1;
  else if (!strcmp(CHAR(STRING_ELT(type, 0)), "nocb"))
    itype = 2;
  else
    error(_("Internal error: invalid type argument in nafillR function, should have been caught before. Please report to data.table issue tracker.")); // # nocov

  bool *isInt64 = (bool *)R_alloc(nx, sizeof(bool));
  for (R_len_t i=0; i<nx; i++)
    isInt64[i] = Rinherits(VECTOR_ELT(x, i), char_integer64);
  const void **fillp = (const void **)R_alloc(nx, sizeof(void*)); // fill is (or will be) a list of length nx of matching types, scalar values for each column, this pointer points to each of those columns data pointers
  bool hasFill = true, badFill = false;
  if (isLogical(fill)) {
    if (length(fill)!=1)
      badFill = true;
    else if (LOGICAL(fill)[0]==NA_LOGICAL) // fill=NA makes hasFill=false
      hasFill = false;
  }
  if (!badFill && hasFill) {
    //Rprintf("hasFill branch\n");
    if (obj_scalar && isNewList(fill))
      badFill = true;
    if (!badFill && !isNewList(fill) && length(fill)!=1)
      badFill = true;
    if (!badFill && !obj_scalar && isNewList(fill) && nx!=length(fill))
      badFill = true;
    if (!badFill && isNewList(fill)) { // each element in fill=list(...) must be length 1
      for (R_len_t i=0; i<length(fill) && !badFill; i++) {
        SEXP thisFill = VECTOR_ELT(fill, i);
        if (isNewList(thisFill) || length(thisFill)!=1)
          badFill = true;
      }
    }
  }
  if (badFill) {
    error(_("fill must be a vector of length 1 or, if x is a list-like, then list of length of x having length 1 elements to fill to corresponding fields"));
  } else if (hasFill) {
    if (!isNewList(fill)) {
      SEXP fill1 = fill;
      fill = PROTECT(allocVector(VECSXP, nx)); protecti++;
      for (int i=0; i<nx; ++i)
        SET_VECTOR_ELT(fill, i, fill1);
    }
    if (!isNewList(fill))
      error("internal error: 'fill' should be recycled as list already"); // # nocov
    //Rprintf("hasFill branch2\n");
    for (R_len_t i=0; i<nx; i++) {
      //Rprintf("coercing fill[i]=\n");
      //Rf_PrintValue(VECTOR_ELT(fill, i));
      //Rprintf("to match x=\n");
      //Rf_PrintValue(VECTOR_ELT(x, i));
      SET_VECTOR_ELT(fill, i, coerceAs(VECTOR_ELT(fill, i), VECTOR_ELT(x, i), ScalarLogical(TRUE)));
      if (isFactor(VECTOR_ELT(x, i)) && hasFill)
        error("'fill' on factor columns is not yet implemented");
      fillp[i] = SEXPPTR_RO(VECTOR_ELT(fill, i)); // do like this so we can use in parallel region
    }
    //Rprintf("hasFill branch3\n");
  }
  //Rprintf("before loop\n");
  //Rf_PrintValue(ans);
  //Rf_PrintValue(VECTOR_ELT(ans, 0));
  //Rprintf("TYPEOF(ans)=%s\n", type2char(TYPEOF(VECTOR_ELT(ans, 0))));
  //Rprintf("before loop fill\n");
  //Rf_PrintValue(VECTOR_ELT(fill, 0));
  #pragma omp parallel for if (nx>1 && !hadChar) num_threads(getDTthreads(nx, true))
  for (R_len_t i=0; i<nx; i++) {
    switch (TYPEOF(VECTOR_ELT(x, i))) {
    case REALSXP : {
      if (isInt64[i]) {
        nafillInteger64(i64x[i], inx[i], itype, hasFill ? ((int64_t *)fillp[i])[0] : NA_INTEGER64, &vans[i], verbose);
      } else {
        nafillDouble(dx[i], inx[i], itype, hasFill ? ((double *)fillp[i])[0] : NA_REAL, nan_is_na, &vans[i], verbose);
      }
    } break;
    case LGLSXP: case INTSXP : {
      nafillInteger(ix[i], inx[i], itype, hasFill ? ((int32_t *)fillp[i])[0] : NA_INTEGER, &vans[i], verbose);
    } break;
    case STRSXP : {
      nafillString(sx[i], inx[i], itype, hasFill ? ((SEXP *)fillp[i])[0] : NA_STRING, &vans[i], verbose);
    } break;
    }
  }

  //Rprintf("after loop\n");
  //Rprintf("TYPEOF(ans)=%s\n", type2char(TYPEOF(VECTOR_ELT(ans, 0))));
  //Rf_PrintValue(VECTOR_ELT(ans, 0));

  if (!binplace) {
    for (R_len_t i=0; i<nx; i++) {
      if (!isNull(ATTRIB(VECTOR_ELT(x, i)))) {
        copyMostAttrib(VECTOR_ELT(x, i), VECTOR_ELT(ans, i));
        if (hasFill && isFactor(VECTOR_ELT(ans, i))) {
          error("TODO merge fill to ans factor level");
        }
      }
    }
    SEXP obj_names = getAttrib(obj, R_NamesSymbol); // copy names
    if (!isNull(obj_names)) {
      SEXP ans_names = PROTECT(allocVector(STRSXP, length(ans))); protecti++;
      for (int i=0; i<length(ricols); i++)
        SET_STRING_ELT(ans_names, i, STRING_ELT(obj_names, icols[i]-1));
      setAttrib(ans, R_NamesSymbol, ans_names);
    }
  }
  //Rprintf("before ansMsg\n");
  ansMsg(vans, nx, verbose, __func__);

  if (verbose)
    Rprintf(_("%s: parallel processing of %d column(s) took %.3fs\n"), __func__, nx, omp_get_wtime()-tic);
  //Rprintf("before return\n");
  UNPROTECT(protecti);
  if (binplace) {
    return obj;
  } else {
    //Rprintf("returning VECTOR_ELT(ans, 0)\n");
    //Rf_PrintValue(VECTOR_ELT(ans, 0));
    return obj_scalar && length(ans) == 1 ? VECTOR_ELT(ans, 0) : ans;
  }
}
