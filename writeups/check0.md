Checkpoint 0 Writeup
====================

My name: [Kesu peterill]

My SUNet ID: [I am not stanford CS student]

I collaborated with: [myself]

I would like to credit/thank these classmates for their help: [myself]

This lab took me about [8] hours to do. I [did not] attend the lab session.

My secret code from section 2.1 was: [853356]

I was surprised by or edified to learn that: [c++ std::string's append is effcient than '+' operator in the byte_stream/push() function as it directly adds up the element from a substring pointer instead of a copy of substring]

Describe ByteStream implementation. [Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

In this lab, I have tried 4 different types of data structure as the buffer.(std::string, std::deque<string>, std::vector<char>, std::queue<char>)

std::string(good):
- compatible with return type of peek(), string_view. string_view is a pointer that points to the first char of the buffer's char array(if the buffer data structure has a char array).
- pop is O(1) in my view, I never test.  I use string view to get the pointer that points to the substring of the buffer's data. Then points buffer to this substring.

std::string(bad):
- O(N) for pushing N size of data.

std::deque<string>(good):
- O(1) for push() as we can just push the input string "data" pointer to the back of the buffer.

std::deque<string>(bad):
- The pop function can take O(N) if the string pushed are scattered small pieces. Exp: buf-> "a" -> "b" -> "c", the runtime for pop(3) will be 3 which is N times.
- Not compatible with the return type of peek(), string_view. Will need many more operations to convert it to string_view.

std::vector<char>:
- similar to std::string but not efficient at pop().

std::queue<char>:
- slower than deque and not compatible with string_view.

- Optional: I had unexpected difficulty with: [choosing the data structure]

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: [I never test other data structures, there could be a data strcture that is more suitable for byte_stream]

- Optional: I contributed a new test case that catches a plausible bug
  not otherwise caught: [I didn't find a new test case]
