array=new Array(3,2,1,10,9,7,5,100,1000)
function sort(array)
{
 for(i=1;i<array.length;i++)
    {
     j=i-1;
     temp=array[i];
     while(j>=0&&array[j]>temp)
          {
           if(temp<array[j])
             {
              array[j+1]=array[j];
              j--;
             }
          }
     array[j+1]=temp;
    }
}
var a=0;
sort(array)

while(a<-1)
      j++;
