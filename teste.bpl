function f1 pi1
def
var vi1
vet va2 size ci10
var vi3
enddef
vi1 = ci3
vi3 = ci4
if vi1 ne vi3 
vi1 = ci0
endif
if vi1 lt vi3 
get va2 index ci8 to vi1
endif
if vi3 le vi1 
return vi1
endif
if pi1 le ci10 
return ci0
endif
if ci-10 gt ci10 
get va2 index ci2 to vi3
endif
return ci-1
end

function f2 pa1
def
reg vr1
var vi2
vet va3 size ci30
enddef
vr1 = ci1
vi2 = call f1 vr1 va3
vi2 = call f1 ci5 pa1
get pa1 index ci3 to vr1
get pa1 index ci0 to vi2
get va3 index ci5 to vr1
get va3 index ci11 to vi2
return vi2
end

function f3 pa1 pi2 pa3
def
vet va1 size ci10
vet va2 size ci20
var vi3
reg vr4
var vi5
reg vr6
enddef
return ci0
end
