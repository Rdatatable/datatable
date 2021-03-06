## Functions to let data.table play nicer with S4
if ("package:data.table" %in% search()) stopf("data.table package loaded. When developing don't load package")

## Allows data.table to be defined as an object of an S4 class,
## or even have data.table be a super class of an S4 class.
setOldClass(c('data.frame'))
setOldClass(c('data.table', 'data.frame'))

## as(some.data.frame, "data.table")
setAs("data.frame", "data.table", function(from) {
  as.data.table(from)
})

## as(some.data.table, "data.frame")
setAs("data.table", "data.frame", function(from) {
  as.data.frame(from)
})

setOldClass("IDate")
setOldClass("ITime")

setAs("character", "IDate", function(from) as.IDate(from))
setAs("character", "ITime", function(from) as.ITime(from))
