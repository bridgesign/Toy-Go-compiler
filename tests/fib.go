fprev:=0
fnext:=1
n:=15
m:=1
while(m<n)
{
	temp := fprev
fprev=fnext
fnext=temp+fnext
m=m+1
}
print fnext
