DESIGN LOG: PROJECT 3
*I used gemini to help compile and run the code on wsl terminal, and my own code from P2
Hash
I decided to use separate chaining. The hash table is a vector composed of buckets, which are vectors, that store unique pointers to each tool called std::unique_ptr<Tool>.  If there is a collision a simple push_back is used keeping it to an O(1) complexity, the find command jumps to the bucket and scans the list, and nothing breaks if a bucket gets crowded. I chose not to use open addressing because it was harder to implement.
The hash is a polynomial string hash: start at 0 and, for every character, compute h = h * 31 + (unsigned char)c. It uses every character and depends on their order. The table starts with 32 buckets. Before each insert, if (size + 1) / buckets > 0.75, add doubles the bucket count and re-inserts every tool at hash % new_size. The load factor therefore never exceeds 0.75 by increasing the total buckets before the size/buckets would be greater than .75.
My test of inserting 10,000 items tool_0 -> tool_9999 resulted in:
My hash (base 31): had a bucket_count = 16384,a load_factor = 0.610,and a longest chain = 5
Weak hash s[0] + s.size() (at 16384 buckets): had a bucket_count = 16384, a load_factor = 0.610, and a longest chain = 9000
My hash out performs the weak hash provided because when testing the weak hash with every name starting with 't' and has the length of 6 to 9, it produces only four distinct values. Of the 10,000 names, 9,000 land in a single chain, so find worsens from a handful of comparisons to thousands. 
The add function is amortized O(1) because ideally and most often the tool key is turned into an index and then a simple push back function is called to append the tool at the end of the bucket and worst case scenario of O(n) only happens once every time the load factor is reached where each element needs rehashed. Since this is so infrequent and will only become less frequent each time it happens the  amortized complexity is O(1).  

Pointer stability
Each bucket holds a vector of unique_ptr, so every Tool lives on the heap at its own address. A resize only moves the unique_ptr values from the old vectors to the new ones, not the tools themselves. This comes with the added bonus of making a rehash faster since it's only moving one pointer compared to an entire object's worth of information. So after a rehash the unique_ptr’s still point to their respective tools in the heap, just now the pointer lives somewhere else.
Calculator trace of 2 * (3 + 4) - 5
push number “2” -> values: [2], ops: []
ops empty, so push “*” -> values: [2], ops: [*]
push “(“ -> values: [2], ops: [*, (]
push number -> values: [2, 3], ops: [*, (]
top is “(“ which has a precedence 0 and 0 is not >= 1, so push + -> values: [2, 3],       ops: [*, (, +]
push number “4”-> values: [2, 3, 4], ops: [*, (, +]
Next input is “)” so compute (3 + 4 = 7) by popping 3 and 4 then pushing 7; top is (, so pop it -> values: [2, 7], ops: [*]
top is “*” which has a precedence of 2 which is >= 1 compared to the “-”, so apply 2 * 7 = 14; push - -> values: [14], ops: [-]
push number -> values: [14, 5], ops: [-]
Lastly apply - (14 - 5 = 9) -> values: [9], ops: []

Errors as values
It is expected that when running an AI agent or any program will run into errors such as dividing by zero, and instead of causing a crash or an exception to instead have an object that reports back the error and in this case let the user and the AI agent know what went wrong and try again. This can keep the work flow smooth while preventing serious issues such as dividing by zero or accessing files that aren’t supposed to be accessed or altered. Then if there is a truly serious issue that is unintended the system can cause an exception and prevent anything going bad.

What I would do Differently
It was hard to wrap my head around the calculator_eval.cpp logic and took a lot of troubleshooting. Next time I will hand write and draw the logic instead of keeping it up in my head. Also for the Hash table, next time instead I may try to have a variable called std::size_t longestbucket that stays updated with the longest chain in its buckets, this would make the  max_probe_length function complexity be O(1) instead of O(n) since it would just return a known value like .size on a vector.
