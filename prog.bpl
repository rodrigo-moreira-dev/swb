function f1 pa1 pi2
def
var vi1
enddef
get pa1 index ci1 to vi1
vi1 = vi1 + ci1
return vi1
end

function f2 pi1
def
var vi1
var vi2
enddef
vi1 = ci1                            
vi2 = vi1
vi1 = pi1 + vi2
vi2 = vi1 * ci-5
return vi1
end

function f3 pi1 pa2
def
enddef
return pi1
end

function f4 pa1
def
var vi1
var vi2
vet va3 size ci30
enddef
vi1 = ci1
vi2 = call f3 vi1 va3
vi2 = call f3 ci5 pa1
return vi2
end

function f5 pi1 pa2
def
var vi1
vet va2 size ci10
enddef
vi1 = pi1 + ci1
set va2 index ci5 with ci2
set pa2 index ci0 with vi1
get va2 index ci8 to vi1
return pi1
end