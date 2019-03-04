#include "data.table.h"
#include <Rdefines.h>


// factorType is 1 for factor and 2 for ordered
// will simply unique normal factors and attempt to find global order for ordered ones
SEXP combineFactorLevels(SEXP factorLevels, int * factorType, Rboolean * isRowOrdered) {
  // find total length
  RLEN size = 0;
  R_len_t len = LENGTH(factorLevels), n, i, j;
  for (i = 0; i < len; ++i) {
    SEXP elem = VECTOR_ELT(factorLevels, i);
    n = LENGTH(elem);
    size += n;
    /* for (j = 0; j < n; ++j) { */
    /*     if(IS_BYTES(STRING_ELT(elem, j))) { */
    /*         data.useUTF8 = FALSE; break; */
    /*     } */
    /*     if(ENC_KNOWN(STRING_ELT(elem, j))) { */
    /*         data.useUTF8 = TRUE; */
    /*     } */
    /*     if(!IS_CACHED(STRING_ELT(elem, j))) { */
    /*         data.useCache = FALSE; break; */
    /*     } */
    /* } */
  }

  // set up hash to put duplicates in
  HashData data;
  data.useUTF8 = FALSE;
  data.useCache = TRUE;
  HashTableSetup(&data, size);

  struct llist **h = data.HashTable;
  hlen idx;
  struct llist * pl;
  R_len_t uniqlen = 0;
  // we insert in opposite order because it's more convenient later to choose first of the duplicates
  for (i = len-1; i >= 0; --i) {
    SEXP elem = VECTOR_ELT(factorLevels, i);
    n = LENGTH(elem);
    for (j = n-1; j >= 0; --j) {
      idx = data.hash(elem, j, &data);
      while (h[idx] != NULL) {
        pl = h[idx];
        if (data.equal(VECTOR_ELT(factorLevels, pl->i), pl->j, elem, j))
          break;
        // it's a collision, not a match, so iterate to a new spot
        idx = (idx + 1) % data.M;
      }
      if (data.nmax-- < 0) error("hash table is full");

      pl = (struct llist *)R_alloc(1, sizeof(struct llist));
      pl->next = NULL;
      pl->i = i;
      pl->j = j;
      if (h[idx] != NULL) {
        pl->next = h[idx];
      } else {
        ++uniqlen;
      }
      h[idx] = pl;
    }
  }

  SEXP finalLevels = PROTECT(allocVector(STRSXP, uniqlen)); // UNPROTECTed at the end of this function
  R_len_t counter = 0;
  if (*factorType == 2) {
    int *locs = (int *)R_alloc(len, sizeof(int));
    for (int i=0; i<len; i++) locs[i] = 0;
    // note there's a goto (!!) normalFactor below. When locs was allocated with malloc, the goto jumped over the
    // old free() and caused leak. Now uses the safer R_alloc.  TODO - review all this logic.

    R_len_t k;
    SEXP tmp;
    for (i = 0; i < len; ++i) {
      if (!isRowOrdered[i]) continue;
      SEXP elem = VECTOR_ELT(factorLevels, i);
      n = LENGTH(elem);
      for (j = locs[i]; j < n; ++j) {
        idx = data.hash(elem, j, &data);
        while (h[idx] != NULL) {
          pl = h[idx];
          if (data.equal(VECTOR_ELT(factorLevels, pl->i), pl->j, elem, j)) {
            do {
              if (!isRowOrdered[pl->i]) continue;

              tmp = VECTOR_ELT(factorLevels, pl->i);
              if (locs[pl->i] > pl->j) {
                // failed to construct global order, need to break out of too many loops
                // so will use goto :o
                warning("ordered factor levels cannot be combined, going to convert to simple factor instead");
                counter = 0;
                *factorType = 1;
                goto normalFactor;
              }

              for (k = locs[pl->i]; k < pl->j; ++k) {
                SET_STRING_ELT(finalLevels, counter++, STRING_ELT(tmp, k));
              }
              locs[pl->i] = pl->j + 1;
            } while ( (pl = pl->next) ); // added parenthesis to remove compiler warning 'suggest parentheses around assignment used as truth value'
            SET_STRING_ELT(finalLevels, counter++, STRING_ELT(elem, j));
            break;
          }
          // it's a collision, not a match, so iterate to a new spot
          idx = (idx + 1) % data.M;
        }
        if (h[idx] == NULL) error("internal hash error, please report to data.table issue tracker");
      }
    }

    // fill in the rest of the unordered elements
    Rboolean record;
    for (i = 0; i < len; ++i) {
      if (isRowOrdered[i]) continue;
      SEXP elem = VECTOR_ELT(factorLevels, i);
      n = LENGTH(elem);
      for (j = 0; j < n; ++j) {
        idx = data.hash(elem, j, &data);
        while (h[idx] != NULL) {
          pl = h[idx];
          if (data.equal(VECTOR_ELT(factorLevels, pl->i), pl->j, elem, j)) {
            // Fixes #899. "rest" can have identical levels in
            // more than 1 data.table.
            if (!(pl->i == i && pl->j == j)) break;
            record = TRUE;
            do {
              // if this element was in an ordered list, it's been recorded already
              if (isRowOrdered[pl->i]) {
                record = FALSE;
                break;
              }
            } while ( (pl = pl->next) ); // added parenthesis to remove compiler warning 'suggest parentheses around assignment used as truth value'
            if (record)
              SET_STRING_ELT(finalLevels, counter++, STRING_ELT(elem, j));

            break;
          }
          // it's a collision, not a match, so iterate to a new spot
          idx = (idx + 1) % data.M;
        }
        if (h[idx] == NULL) error("internal hash error, please report to data.table issue tracker");
      }
    }
  }

 normalFactor:
  if (*factorType == 1) {
    for (i = 0; i < len; ++i) {
      SEXP elem = VECTOR_ELT(factorLevels, i);
      n = LENGTH(elem);
      for (j = 0; j < n; ++j) {
        idx = data.hash(elem, j, &data);
        while (h[idx] != NULL) {
          pl = h[idx];
          if (data.equal(VECTOR_ELT(factorLevels, pl->i), pl->j, elem, j)) {
            if (pl->i == i && pl->j == j) {
              SET_STRING_ELT(finalLevels, counter++, STRING_ELT(elem, j));
            }
            break;
          }
          // it's a collision, not a match, so iterate to a new spot
          idx = (idx + 1) % data.M;
        }
        if (h[idx] == NULL) error("internal hash error, please report to data.table issue tracker");
      }
    }
  }

  // CleanHashTable(&data);   No longer needed now we use R_alloc(). But the hash table approach
  // will be removed completely at some point.
  UNPROTECT(1); // finalLevels
  return finalLevels;
}


/* Arun's addition and changes to incorporate 'usenames=T/F' and 'fill=T/F' arguments to rbindlist */

/*
  l               = input list of data.tables/lists/data.frames
  n               = length(l)
  ans             = rbind'd result
  i               = an index over length of l
  use.names       = whether binding should check for names and bind them accordingly
  fill            = whether missing columns should be filled with NAs

  ans_ptr         = final_names - column names for 'ans' (list item 1)
  ans_ptr         = match_indices - when use.names=TRUE, for each element in l, what's the destination col index (in 'ans') for each of the cols (list item 2)
  n_rows          = total number of rows in 'ans'
  n_cols          = total number of cols in 'ans'
  max_type        = for each col in 'ans', what's the final SEXPTYPE? (for coercion if necessary)
  is_factor       = for each col in 'ans' mark which one's a factor (to convert to factor at the end)
  is_ofactor      = for each col in 'ans' mark which one's an ordered factor (to convert to ordered factor at the end)
  fn_rows         = the length of first column (rows) for each item in l.
  mincol          = get the minimum number of columns in an item from l. Used to check if 'fill=TRUE' is really necessary, even if set.
*/

struct preprocessData {
  SEXP ans_ptr, colname;
  size_t n_rows, n_cols;
  int *fn_rows, *is_factor, first, lcount, mincol, protecti;
  SEXPTYPE *max_type;
};

static SEXP unlist2(SEXP v) {

  RLEN i, j, k=0, ni, n=0;
  SEXP ans, vi, lnames, groups, runids;

  for (i=0; i<length(v); i++) n += length(VECTOR_ELT(v, i));
  ans    = PROTECT(allocVector(VECSXP, 3));
  lnames = PROTECT(allocVector(STRSXP, n));
  groups = PROTECT(allocVector(INTSXP, n));
  runids = PROTECT(allocVector(INTSXP, n));
  for (i=0; i<length(v); i++) {
    vi = VECTOR_ELT(v, i);
    ni = length(vi);
    for (j=0; j<ni; j++) {
      SET_STRING_ELT(lnames, k + j, STRING_ELT(vi, j));
      INTEGER(groups)[k + j] = i+1;
      INTEGER(runids)[k + j] = j;
    }
    k+=j;
  }
  SET_VECTOR_ELT(ans, 0, lnames);
  SET_VECTOR_ELT(ans, 1, groups);
  SET_VECTOR_ELT(ans, 2, runids);
  UNPROTECT(4);
  return(ans);
}

// Don't use elsewhere. No checks are made on byArg and handleSorted
// if handleSorted is 0, then it'll return integer(0) as such when
// input is already sorted, like forder. if not, seq_len(nrow(dt)).
static SEXP fast_order(SEXP dt, R_len_t byArg, R_len_t handleSorted) {

  R_len_t i, protecti=0;
  SEXP ans, by=R_NilValue, retGrp, sortStr, order, na, starts;

  retGrp  = PROTECT(allocVector(LGLSXP, 1)); LOGICAL(retGrp)[0]  = TRUE;   protecti++;
  sortStr = PROTECT(allocVector(LGLSXP, 1)); LOGICAL(sortStr)[0] = FALSE;  protecti++;
  na      = PROTECT(allocVector(LGLSXP, 1)); LOGICAL(na)[0] = FALSE;       protecti++;

  if (byArg) {
    by    = PROTECT(allocVector(INTSXP, byArg));                           protecti++;
    order = PROTECT(allocVector(INTSXP, byArg));                           protecti++;
    for (i=0; i<byArg; i++) {
      INTEGER(by)[i] = i+1;
      INTEGER(order)[i] = 1;
    }
  } else {
    order = PROTECT(allocVector(INTSXP, 1)); INTEGER(order)[0] = 1;        protecti++;
  }
  ans = PROTECT(forder(dt, by, retGrp, sortStr, order, na));               protecti++;
  if (!length(ans) && handleSorted != 0) {
    starts = PROTECT(getAttrib(ans, sym_starts));                          protecti++;
    // if cols are already sorted, 'forder' gives integer(0), got to replace it with 1:.N
    ans = PROTECT(allocVector(INTSXP, length(VECTOR_ELT(dt, 0))));         protecti++;
    for (i=0; i<length(ans); i++) INTEGER(ans)[i] = i+1;
    // TODO: for loop appears redundant because length(ans)==0 due to if (!length(ans)) above
    setAttrib(ans, sym_starts, starts);
  }
  UNPROTECT(protecti);
  return(ans);
}

static SEXP uniq_lengths(SEXP v, R_len_t n) {
  R_len_t nv=length(v);
  SEXP ans = PROTECT(allocVector(INTSXP, nv));
  for (R_len_t i=1; i<nv; i++) {
    INTEGER(ans)[i-1] = INTEGER(v)[i] - INTEGER(v)[i-1];
  }
  if (nv>0) {
    // last value
    INTEGER(ans)[nv-1] = n - INTEGER(v)[nv-1] + 1;
  }
  UNPROTECT(1);
  return(ans);
}

static SEXP match_names(SEXP v) {

  R_len_t i, j, idx, ncols, protecti=0;
  SEXP ans, dt, lnames, ti;
  SEXP uorder, starts, ulens, index, firstofeachgroup, origorder;
  SEXP fnames, findices, runid, grpid;

  ans    = PROTECT(allocVector(VECSXP, 2));
  dt     = PROTECT(unlist2(v)); protecti++;
  lnames = VECTOR_ELT(dt, 0);
  grpid  = PROTECT(duplicate(VECTOR_ELT(dt, 1))); protecti++; // dt[1] will be reused, so backup
  runid  = VECTOR_ELT(dt, 2);

  uorder = PROTECT(fast_order(dt, 2, 1));  protecti++; // byArg alone is set, everything else is set inside fast_order
  starts = getAttrib(uorder, sym_starts);
  ulens  = PROTECT(uniq_lengths(starts, length(lnames))); protecti++;

  // seq_len(.N) for each group
  index = PROTECT(VECTOR_ELT(dt, 1)); protecti++; // reuse dt[1] (in 0-index coordinate), value already backed up above.
  for (i=0; i<length(ulens); i++) {
    for (j=0; j<INTEGER(ulens)[i]; j++)
      INTEGER(index)[INTEGER(uorder)[INTEGER(starts)[i]-1+j]-1] = j;
  }
  // order again
  uorder = PROTECT(fast_order(dt, 2, 1));  protecti++; // byArg alone is set, everything else is set inside fast_order
  starts = getAttrib(uorder, sym_starts);
  ulens  = PROTECT(uniq_lengths(starts, length(lnames))); protecti++;
  ncols  = length(starts);
  // check if order has to be changed (bysameorder = FALSE here by default - in `[.data.table` parlance)
  firstofeachgroup = PROTECT(allocVector(INTSXP, length(starts)));
  for (i=0; i<ncols; i++) INTEGER(firstofeachgroup)[i] = INTEGER(uorder)[INTEGER(starts)[i]-1];
  origorder = PROTECT(fast_order(firstofeachgroup, 0, 0));
  if (length(origorder)) {
    reorder(starts, origorder);
    reorder(ulens, origorder);
  }
  UNPROTECT(2);
  // get fnames and findices
  fnames   = PROTECT(allocVector(STRSXP, ncols)); protecti++;
  findices = PROTECT(allocVector(VECSXP, ncols)); protecti++;
  for (i=0; i<ncols; i++) {
    idx = INTEGER(uorder)[INTEGER(starts)[i]-1]-1;
    SET_STRING_ELT(fnames, i, STRING_ELT(lnames, idx));
    ti = PROTECT(allocVector(INTSXP, length(v)));
    for (j=0;j<length(v);j++) INTEGER(ti)[j]=-1; // TODO: can we eliminate this?
    for (j=0; j<INTEGER(ulens)[i]; j++) {
      idx = INTEGER(uorder)[INTEGER(starts)[i]-1+j]-1;
      INTEGER(ti)[INTEGER(grpid)[idx]-1] = INTEGER(runid)[idx];
    }
    UNPROTECT(1);
    SET_VECTOR_ELT(findices, i, ti);
  }
  UNPROTECT(protecti);
  SET_VECTOR_ELT(ans, 0, fnames);
  SET_VECTOR_ELT(ans, 1, findices);
  UNPROTECT(1); // ans
  return(ans);
}

static void preprocess(SEXP l, Rboolean usenames, Rboolean fill, struct preprocessData *data) {

  R_len_t i, j, idx;
  SEXP li, lnames=R_NilValue, fnames, findices=R_NilValue, f_ind=R_NilValue, thiscol, col_name=R_NilValue, thisClass = R_NilValue;
  SEXPTYPE type;

  data->first = -1; data->lcount = 0; data->n_rows = 0; data->n_cols = 0; data->protecti = 0;
  data->max_type = NULL; data->is_factor = NULL; data->ans_ptr = R_NilValue; data->mincol=0;
  data->fn_rows = (int *)R_alloc(LENGTH(l), sizeof(int));
  data->colname = R_NilValue;

  // get first non null name, 'rbind' was doing a 'match.names' for each item.. which is a bit more time consuming.
  // And warning that it'll be matched by names is not necessary, I think, as that's the default for 'rbind'. We
  // should instead document it.
  for (i=0; i<LENGTH(l); i++) { // isNull is checked already in rbindlist
    li = VECTOR_ELT(l, i);
    if (isNull(li)) continue;
    if (TYPEOF(li) != VECSXP) error("Item %d of list input is not a data.frame, data.table or list",i+1);
    if (!LENGTH(li)) continue;
    col_name = getAttrib(li, R_NamesSymbol);
    if (!isNull(col_name)) break;
  }
  if (!isNull(col_name)) { data->colname = PROTECT(col_name); data->protecti++; }
  if (usenames) { lnames = PROTECT(allocVector(VECSXP, LENGTH(l))); data->protecti++;}
  for (i=0; i<LENGTH(l); i++) {
    data->fn_rows[i] = 0;  // careful to initialize before continues as R_alloc above doesn't initialize
    li = VECTOR_ELT(l, i);
    if (isNull(li)) continue;
    if (TYPEOF(li) != VECSXP) error("Item %d of list input is not a data.frame, data.table or list",i+1);
    if (!LENGTH(li)) continue;
    col_name = getAttrib(li, R_NamesSymbol);
    if (fill && isNull(col_name))
      error("fill=TRUE, but names of input list at position %d is NULL. All items of input list must have names set when fill=TRUE.", i+1);
    data->lcount++;
    data->fn_rows[i] = length(VECTOR_ELT(li, 0));
    if (data->first == -1) {
      data->first = i;
      data->n_cols = LENGTH(li);
      data->mincol = LENGTH(li);
      if (!usenames) {
        data->ans_ptr = PROTECT(allocVector(VECSXP, 2)); data->protecti++;
        if (isNull(col_name)) SET_VECTOR_ELT(data->ans_ptr, 0, data->colname);
        else SET_VECTOR_ELT(data->ans_ptr, 0, col_name);
      } else {
        if (isNull(col_name)) SET_VECTOR_ELT(lnames, i, data->colname);
        else SET_VECTOR_ELT(lnames, i, col_name);
      }
      data->n_rows += data->fn_rows[i];
      continue;
    } else {
      if (!fill && LENGTH(li) != data->n_cols)
        if (LENGTH(li) != data->n_cols) error("Item %d has %d columns, inconsistent with item %d which has %d columns. If instead you need to fill missing columns, use set argument 'fill' to TRUE.",i+1, LENGTH(li), data->first+1, data->n_cols);
    }
    if (data->mincol > LENGTH(li)) data->mincol = LENGTH(li);
    data->n_rows += data->fn_rows[i];
    if (usenames) {
      if (isNull(col_name)) SET_VECTOR_ELT(lnames, i, data->colname);
      else SET_VECTOR_ELT(lnames, i, col_name);
    }
  }
  if (usenames) {
    data->ans_ptr = PROTECT(match_names(lnames)); data->protecti++;
    fnames = VECTOR_ELT(data->ans_ptr, 0);
    findices = VECTOR_ELT(data->ans_ptr, 1);
    if (isNull(data->colname) && data->n_cols > 0)
      error("use.names=TRUE but no item of input list has any names.\n");
    if (!fill && length(fnames) != data->mincol) {
      error("Answer requires %d columns whereas one or more item(s) in the input list has only %d columns. This could be because the items in the list may not all have identical column names or some of the items may have duplicate names. In either case, if you're aware of this and would like to fill those missing columns, set the argument 'fill=TRUE'.", length(fnames), data->mincol);
    } else data->n_cols = length(fnames);
  }

  // decide type of each column
  // initialize the max types - will possibly increment later
  data->max_type  = (SEXPTYPE *)R_alloc(data->n_cols, sizeof(SEXPTYPE));
  data->is_factor = (int *)R_alloc(data->n_cols, sizeof(int));
  for (i = 0; i< data->n_cols; i++) {
    thisClass = R_NilValue;
    data->max_type[i] = 0;
    data->is_factor[i] = 0;
    if (usenames) f_ind = VECTOR_ELT(findices, i);
    for (j=data->first; j<LENGTH(l); j++) {
      if (data->is_factor[i] == 2) break;
      idx = (usenames) ? INTEGER(f_ind)[j] : i;
      li = VECTOR_ELT(l, j);
      if (isNull(li) || !LENGTH(li) || idx < 0) continue;
      thiscol = VECTOR_ELT(li, idx);
      // Fix for #705, check attributes
      if (j == data->first)
        thisClass = getAttrib(thiscol, R_ClassSymbol);
      if (isFactor(thiscol)) {
        data->is_factor[i] = (isOrdered(thiscol)) ? 2 : 1;
        data->max_type[i]  = STRSXP;
      } else {
        // Fix for #705, check attributes and error if non-factor class and not identical
        if (!data->is_factor[i] &&
          !R_compute_identical(thisClass, getAttrib(thiscol, R_ClassSymbol), 0) && !fill) {
          error("Class attributes at column %d of input list at position %d does not match with column %d of input list at position %d. Coercion of objects of class 'factor' alone is handled internally by rbind/rbindlist at the moment.", i+1, j+1, i+1, data->first+1);
        }
        type = TYPEOF(thiscol);
        if (type > data->max_type[i]) data->max_type[i] = type;
      }
    }
  }
}

// function does c(idcol, nm), where length(idcol)=1
// fix for #1432, + more efficient to move the logic to C
SEXP add_idcol(SEXP nm, SEXP idcol, int cols) {
  SEXP ans = PROTECT(allocVector(STRSXP, cols+1));
  SET_STRING_ELT(ans, 0, STRING_ELT(idcol, 0));
  for (int i=0; i<cols; i++) {
    SET_STRING_ELT(ans, i+1, STRING_ELT(nm, i));
  }
  UNPROTECT(1);
  return (ans);
}





SEXP rbindlist(SEXP l, SEXP sexp_usenames, SEXP sexp_fill, SEXP idcol) {

  R_len_t jj, ansloc, resi, i,j, idx, thislen;
  struct preprocessData data;
  Rboolean to_copy = FALSE, coerced=FALSE, isidcol = !isNull(idcol);
  SEXP fnames = R_NilValue, findices = R_NilValue, f_ind = R_NilValue, ans, lf, li, target, thiscol, levels;
  R_len_t protecti=0;

  // first level of error checks
  if (!isLogical(sexp_usenames) || LENGTH(sexp_usenames)!=1 || LOGICAL(sexp_usenames)[0]==NA_LOGICAL)
    error("use.names should be TRUE or FALSE");
  if (!isLogical(sexp_fill) || LENGTH(sexp_fill) != 1 || LOGICAL(sexp_fill)[0] == NA_LOGICAL)
    error("fill should be TRUE or FALSE");
  if (!length(l)) return(l);
  if (TYPEOF(l) != VECSXP) error("Input to rbindlist must be a list of data.tables, data.frames or lists");

  bool usenames = LOGICAL(sexp_usenames)[0];
  bool fill = LOGICAL(sexp_fill)[0];
  if (fill && !usenames) {
    // override default
    warning("Resetting 'use.names' to TRUE. 'use.names' can not be FALSE when 'fill=TRUE'.\n");
    usenames=true;
  }

  // check for factor, get max types, and when usenames=TRUE get the answer 'names' and column indices for proper reordering.
  preprocess(l, usenames, fill, &data);
  if (usenames) findices = VECTOR_ELT(data.ans_ptr, 1);
  protecti = data.protecti;   // TODO very ugly and doesn't seem right. Assign items to list instead, perhaps.
  if (data.n_rows == 0 && data.n_cols == 0) {
    UNPROTECT(protecti);
    return(R_NilValue);
  }
  if (data.n_rows > INT32_MAX) {
    error("Total rows in the list is %lld which is larger than the maximum number of rows, currently %d",
          (long long)data.n_rows, INT32_MAX);
  }
  fnames = VECTOR_ELT(data.ans_ptr, 0);
  if (isidcol) {
    fnames = PROTECT(add_idcol(fnames, idcol, data.n_cols));
    protecti++;
  }
  SEXP factorLevels = PROTECT(allocVector(VECSXP, data.lcount)); protecti++;
  Rboolean *isRowOrdered = (Rboolean *)R_alloc(data.lcount, sizeof(Rboolean));
  for (int i=0; i<data.lcount; i++) isRowOrdered[i] = FALSE;

  ans = PROTECT(allocVector(VECSXP, data.n_cols+isidcol)); protecti++;
  setAttrib(ans, R_NamesSymbol, fnames);
  lf = VECTOR_ELT(l, data.first);
  for(j=0; j<data.n_cols; j++) {
    if (fill) target = allocNAVector(data.max_type[j], data.n_rows);  // no PROTECT needed as passed immediately to SET_VECTOR_ELT
    else target = allocVector(data.max_type[j], data.n_rows);         // no PROTECT needed as passed immediately to SET_VECTOR_ELT
    SET_VECTOR_ELT(ans, j+isidcol, target);

    if (usenames) {
      to_copy = TRUE;
      f_ind   = VECTOR_ELT(findices, j);
    } else {
      thiscol = VECTOR_ELT(lf, j);
      if (!isFactor(thiscol)) copyMostAttrib(thiscol, target); // all but names,dim and dimnames. And if so, we want a copy here, not keepattr's SET_ATTRIB.
    }
    ansloc = 0;
    jj = 0; // to increment factorLevels
    resi = -1;
    for (i=data.first; i<LENGTH(l); i++) {
      li = VECTOR_ELT(l,i);
      if (!length(li)) continue;  // majority of time though, each item of l is populated
      thislen = data.fn_rows[i];
      idx = (usenames) ? INTEGER(f_ind)[i] : j;
      if (idx < 0) {
        ansloc += thislen;
        resi++;
        if (data.is_factor[j]) {
          isRowOrdered[resi] = FALSE;
          SET_VECTOR_ELT(factorLevels, jj, allocNAVector(data.max_type[j], 1)); // the only level here is NA.
          jj++;
        }
        continue;
      }
      thiscol = VECTOR_ELT(li, idx);
      if (thislen != length(thiscol)) error("Column %d of item %d is length %d, inconsistent with first column of that item which is length %d. rbind/rbindlist doesn't recycle as it already expects each item to be a uniform list, data.frame or data.table", j+1, i+1, length(thiscol), thislen);
      // couldn't figure out a way to this outside this loop when fill = TRUE.
      if (to_copy && !isFactor(thiscol)) {
        copyMostAttrib(thiscol, target);
        to_copy = FALSE;
      }
      resi++;  // after the first, there might be NULL or empty which are skipped, resi increments up until lcount
      if (TYPEOF(thiscol) != TYPEOF(target) && !isFactor(thiscol)) {
        thiscol = PROTECT(coerceVector(thiscol, TYPEOF(target)));
        coerced = TRUE;
        // TO DO: options(datatable.pedantic=TRUE) to issue this warning :
        // warning("Column %d of item %d is type '%s', inconsistent with column %d of item %d's type ('%s')",j+1,i+1,type2char(TYPEOF(thiscol)),j+1,first+1,type2char(TYPEOF(target)));
      }
      if (TYPEOF(target)!=STRSXP && TYPEOF(thiscol)!=TYPEOF(target)) {
        error("Internal error in rbindlist.c: type of 'thiscol' [%s] should have already been coerced to 'target' [%s]. please report to data.table issue tracker.",
              type2char(TYPEOF(thiscol)), type2char(TYPEOF(target)));
      }
      switch(TYPEOF(target)) {
      case LGLSXP:
        memcpy(LOGICAL(target)+ansloc, LOGICAL(thiscol), thislen*SIZEOF(thiscol));
        break;
      case INTSXP:
        memcpy(INTEGER(target)+ansloc, INTEGER(thiscol), thislen*SIZEOF(thiscol));
        break;
      case REALSXP:
        memcpy(REAL(target)+ansloc, REAL(thiscol), thislen*SIZEOF(thiscol));
        break;
      case CPLXSXP :
        memcpy(COMPLEX(target)+ansloc, COMPLEX(thiscol), thislen*sizeof(Rcomplex));
        break;
      case VECSXP :
        for (int r=0; r<thislen; r++)
          SET_VECTOR_ELT(target, ansloc+r, VECTOR_ELT(thiscol,r));
        break;
      case STRSXP :
        isRowOrdered[resi] = FALSE;
        if (isFactor(thiscol)) {
          levels = getAttrib(thiscol, R_LevelsSymbol);
          if (isNull(levels)) error("Column %d of item %d has type 'factor' but has no levels; i.e. malformed.", j+1, i+1);
          for (r=0; r<thislen; r++)
            if (INTEGER(thiscol)[r]==NA_INTEGER)
              SET_STRING_ELT(target, ansloc+r, NA_STRING);
            else
              SET_STRING_ELT(target, ansloc+r, STRING_ELT(levels,INTEGER(thiscol)[r]-1));

          // add levels to factorLevels
          // changed "i" to "jj" and increment 'jj' after so as to fill only non-empty tables with levels
          SET_VECTOR_ELT(factorLevels, jj, levels); jj++;
          if (isOrdered(thiscol)) isRowOrdered[resi] = TRUE;
        } else {
          if (TYPEOF(thiscol) != STRSXP) error("Internal logical error in rbindlist.c (not STRSXP), please report to data.table issue tracker.");
          for (int r=0; r<thislen; r++) SET_STRING_ELT(target, ansloc+r, STRING_ELT(thiscol,r));

          // if this column is going to be a factor, add column to factorLevels
          // changed "i" to "jj" and increment 'jj' after so as to fill only non-empty tables with levels
          if (data.is_factor[j]) {
            SET_VECTOR_ELT(factorLevels, jj, thiscol);
            jj++;
          }
          // removed 'coerced=FALSE; UNPROTECT(1)' as it resulted in a stack imbalance.
          // anyways it's taken care of after the switch. So no need here.
        }
        break;
      default :
        error("Unsupported column type '%s'", type2char(TYPEOF(target)));
      }
      ansloc += thislen;
      if (coerced) {
        UNPROTECT(1);
        coerced = FALSE;
      }
    }
    if (data.is_factor[j]) {
      SEXP finalFactorLevels = PROTECT(combineFactorLevels(factorLevels, &(data.is_factor[j]), isRowOrdered));
      SEXP factorLangSxp = PROTECT(lang3(install(data.is_factor[j] == 1 ? "factor" : "ordered"),
                         target, finalFactorLevels));
      SET_VECTOR_ELT(ans, j+isidcol, eval(factorLangSxp, R_GlobalEnv));
      UNPROTECT(2);  // finalFactorLevels, factorLangSxp
    }
  }

  // fix for #1432, + more efficient to move the logic to C
  if (isidcol) {
    R_len_t runidx = 1, cntridx = 0;
    SEXP lnames = getAttrib(l, R_NamesSymbol);
    if (isNull(lnames)) {
      SET_VECTOR_ELT(ans, 0, target=allocVector(INTSXP, data.n_rows) );
      for (i=0; i<LENGTH(l); i++) {
        for (j=0; j<data.fn_rows[i]; j++)
          INTEGER(target)[cntridx++] = runidx;
        runidx++;
      }
    } else {
      SET_VECTOR_ELT(ans, 0, target=allocVector(STRSXP, data.n_rows) );
      for (i=0; i<LENGTH(l); i++) {
        for (j=0; j<data.fn_rows[i]; j++)
          SET_STRING_ELT(target, cntridx++, STRING_ELT(lnames, i));
      }
    }
  }

  UNPROTECT(protecti);
  return(ans);
}

/*
## The section below implements "chmatch2_old" and "chmatch2" (faster version of chmatch2_old).
## It's basically 'pmatch' but without the partial matching part. These examples should
## make it clearer.
## Examples:
## chmatch2_old(c("a", "a"), c("a", "a"))     # 1,2  - the second 'a' in 'x' has a 2nd match in 'table'
## chmatch2_old(c("a", "a"), c("a", "b"))     # 1,NA - the second one doesn't 'see' the first 'a'
## chmatch2_old(c("a", "a"), c("a", "a.1"))   # 1,NA - differs from 'pmatch' output = 1,2
##
## The algorithm: given 'x' and 'y':
## dt = data.table(val=c(x,y), grp1 = rep(1:2, c(length(x),length(y))), grp2=c(1:length(x), 1:length(y)))
## dt[, grp1 := 0:(.N-1), by="val,grp1"]
## dt[, grp2[2], by="val,grp1"]
##
## NOTE: This is FAST, but not AS FAST AS it could be. See chmatch2 for a faster implementation (and bottom
## of this file for a benchmark). I've retained here for now. Ultimately, will've to discuss with Matt and
## probably export it??
*/
SEXP chmatch2_old(SEXP x, SEXP table, SEXP nomatch) {

  R_len_t i, j, k, nx, li, si, oi;
  if (TYPEOF(nomatch) != INTSXP || length(nomatch) != 1) error("'nomatch' must be an integer of length 1");
  if (!length(x) || isNull(x)) return(allocVector(INTSXP, 0));
  if (TYPEOF(x) != STRSXP) error("'x' must be a character vector");
  nx=length(x);
  if (!length(table) || isNull(table)) {
    SEXP ans = PROTECT(allocVector(INTSXP, nx));
    for (i=0; i<nx; i++) INTEGER(ans)[i] = INTEGER(nomatch)[0];
    UNPROTECT(1);
    return(ans);
  }
  if (TYPEOF(table) != STRSXP) error("'table' must be a character vector");
  // Done with special cases. On to the real deal.

  SEXP tt = PROTECT(allocVector(VECSXP, 2));
  SET_VECTOR_ELT(tt, 0, x);
  SET_VECTOR_ELT(tt, 1, table);
  SEXP dt = PROTECT(unlist2(tt));

  // order - first time
  SEXP order = PROTECT(fast_order(dt, 2, 1));
  SEXP start = getAttrib(order, sym_starts);
  SEXP lens  = PROTECT(uniq_lengths(start, length(order))); // length(order) = nrow(dt)
  SEXP grpid = VECTOR_ELT(dt, 1);
  SEXP index = VECTOR_ELT(dt, 2);

  // replace dt[1], we don't need it anymore
  k=0;
  for (i=0; i<length(lens); i++) {
    for (j=0; j<INTEGER(lens)[i]; j++) {
      INTEGER(grpid)[INTEGER(order)[k+j]-1] = j;
    }
    k += j;
  }
  // order - again
  order = PROTECT(fast_order(dt, 2, 1));
  start = getAttrib(order, sym_starts);
  lens  = PROTECT(uniq_lengths(start, length(order)));

  SEXP ans = PROTECT(allocVector(INTSXP, nx));
  k = 0;
  for (i=0; i<length(lens); i++) {
    li = INTEGER(lens)[i];
    si = INTEGER(start)[i]-1;
    oi = INTEGER(order)[si]-1;
    if (oi > nx-1) continue;
    INTEGER(ans)[oi] = (li == 2) ? INTEGER(index)[INTEGER(order)[si+1]-1]+1 : INTEGER(nomatch)[0];
  }
  UNPROTECT(7);
  return(ans);
}

// utility function used from within chmatch2
static SEXP listlist(SEXP x) {

  R_len_t i,j,k, nl;
  SEXP lx, xo, xs, xl, tmp, ans, ans0, ans1;

  lx = PROTECT(allocVector(VECSXP, 1));
  SET_VECTOR_ELT(lx, 0, x);
  xo = PROTECT(fast_order(lx, 1, 1));
  xs = getAttrib(xo, sym_starts);
  xl = PROTECT(uniq_lengths(xs, length(x)));

  ans0 = PROTECT(allocVector(STRSXP, length(xs)));
  ans1 = PROTECT(allocVector(VECSXP, length(xs)));
  k=0;
  for (i=0; i<length(xs); i++) {
    SET_STRING_ELT(ans0, i, STRING_ELT(x, INTEGER(xo)[INTEGER(xs)[i]-1]-1));
    nl = INTEGER(xl)[i];
    SET_VECTOR_ELT(ans1, i, tmp=allocVector(INTSXP, nl) );
    for (j=0; j<nl; j++) {
      INTEGER(tmp)[j] = INTEGER(xo)[k+j];
    }
    k += j;
  }
  ans = PROTECT(allocVector(VECSXP, 2));
  SET_VECTOR_ELT(ans, 0, ans0);
  SET_VECTOR_ELT(ans, 1, ans1);
  UNPROTECT(6);
  return(ans);
}

/*
## While chmatch2_old works great, I find it inefficient in terms of both memory (stores 2 indices over the
## length of x+y) and speed (2 ordering and looping over unnecesssary amount of times). So, here's
## another stab at a faster version of 'chmatch2_old', leveraging the power of 'chmatch' and data.table's
## DT[ , list(list()), by=.] syntax.
##
## The algorithm:
## x.agg = data.table(x)[, list(list(rep(x, .N))), by=x]
## y.agg = data.table(y)[, list(list(rep(y, .N))), by=y]
## mtch  = chmatch(x.agg, y.agg, nomatch)                 ## here we look at only unique values!
## Now, it's just a matter of filling corresponding matches from x.agg's indices with y.agg's indices.
## BENCHMARKS ON THE BOTTOM OF THIS FILE
*/
SEXP chmatch2(SEXP x, SEXP y, SEXP nomatch) {

  R_len_t i, j, k, nx, ix, iy;
  SEXP xll, yll, xu, yu, ans, xl, yl, mx;
  if (TYPEOF(nomatch) != INTSXP || length(nomatch) != 1) error("'nomatch' must be an integer of length 1");
  if (!length(x) || isNull(x)) return(allocVector(INTSXP, 0));
  if (TYPEOF(x) != STRSXP) error("'x' must be a character vector");
  nx = length(x);
  if (!length(y) || isNull(y)) {
    ans = PROTECT(allocVector(INTSXP, nx));
    for (i=0; i<nx; i++) INTEGER(ans)[i] = INTEGER(nomatch)[0];
    UNPROTECT(1);
    return(ans);
  }
  if (TYPEOF(y) != STRSXP) error("'table' must be a character vector");
  // Done with special cases. On to the real deal.
  xll = PROTECT(listlist(x));
  yll = PROTECT(listlist(y));

  xu = VECTOR_ELT(xll, 0);
  yu = VECTOR_ELT(yll, 0);

  mx  = PROTECT(chmatch(xu, yu, 0, FALSE));
  ans = PROTECT(allocVector(INTSXP, nx));
  k=0;
  for (i=0; i<length(mx); i++) {
    xl = VECTOR_ELT(VECTOR_ELT(xll, 1), i);
    ix = length(xl);
    if (INTEGER(mx)[i] == 0) {
      for (j=0; j<ix; j++)
        INTEGER(ans)[INTEGER(xl)[j]-1] = INTEGER(nomatch)[0];
    } else {
      yl = VECTOR_ELT(VECTOR_ELT(yll, 1), INTEGER(mx)[i]-1);
      iy = length(yl);
      for (j=0; j < ix; j++)
        INTEGER(ans)[INTEGER(xl)[j]-1] = (j < iy) ? INTEGER(yl)[j] : INTEGER(nomatch)[0];
      k += ix;
    }
  }
  UNPROTECT(4);
  return(ans);

}

/*
## Benchmark:
set.seed(45L)
x <- sample(letters, 1e6, TRUE)
y <- sample(letters, 1e7, TRUE)
system.time(ans1 <- .Call("Cchmatch2_old", x,y,0L)) # 2.405 seconds
system.time(ans2 <- .Call("Cchmatch2", x,y,0L)) # 0.174 seconds
identical(ans1, ans2) # [1] TRUE
## Note: 'pmatch(x,y,0L)' dint finish still after about 5 minutes, so stopped.
## Speed up of about ~14x!!! nice ;). (And this uses lesser memory as well).
*/
