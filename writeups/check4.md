Checkpoint 3 Writeup
====================

My name: [your name here]

My SUNet ID: [your sunetid here]

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This checkpoint took me about [n] hours to do. I [did/did not] attend the lab session.

Analyzing data:
Time: 2 hour 6 mins(7560s)(37800 packets)
Total msg received: The last ICMP msg in the output data.txt shows the sequence number of 38749 as shown below. I have also wrote a program to calculate the number of unique sequence number in output file "data.txt" using hashset. There are totally 38566 unique sequence numbers.
[1722701187.400738] 64 bytes from 142.250.179.228(www.google.com): icmp_seq=38749 ttl=117 time=308 ms

question 1: What was the overall delivery rate over the entire interval? In other words: how many
echo replies were received, divided by how many echo requests were sent? (Note: ping
on GNU/Linux doesn’t print any message about echo replies that are not received.
You’ll have to identify missing replies by looking for missing sequence numbers.)

ans: The overall delivery rate is 38749/38566 = 99.5%

question 2: What was the longest consecutive string of successful pings (all replied-to in a row)?

ans: I have written an alogorithm and get the max consecutive sequence number is 2172.

question 3: What was the longest burst of losses (all not replied-to in a row)?

ans: The maximum consecutive missed sequence number is 1.

question 4: How independent or correlated is the event of “packet loss” over time? In other words:
• Given that echo request #N received a reply, what is the probability that echo
request #(N+1) was also successfully replied-to?

ans: The average length a consecutive sequenced packet received is 38566/183(missed seqno) = 210. If the n = successful, then the probability that n+1 is successful is 210-1/210 = 99.52%.


• Given that echo request #N did not receive a reply, what is the probability that
echo request #(N+1) was successfully replied-to?

ans: The probability is 100%.


• How do these figures (the conditional delivery rates) compare with the overall
“unconditional” packet delivery rate in the first question? How independent or
bursty were the losses?

ans: The probability of packet loss is not so independent on time. We have also seen a consecutive sequence number of size 2172 though the average consecutive sequence length is 210.


question 5: What was the minimum RTT seen over the entire interval? (This is probably a
reasonable approximation of the true MinRTT...)

ans: The minimum RTT is 242ms. I loop each line in the data.txt and find the lowest time(RTT) which is 242ms.

question 6: What was the maximum RTT seen over the entire interval?

ans: The maximum RTT is 513.

question 7: Make a graph of the RTT as a function of time. Label the x-axis with the actual time of
day (covering the 2+-hour period), and the y-axis should be the number of milliseconds
of RTT.

ans: Since the total numer of data is too much, I can't fix it in a small chart. I will use a time interval of 155*0.2s = 31s instead of 0.2s. The graph is store in the lab4/ directory with name "qn7.png"


question 8: Make a histogram or Cumulative Distribution Function of the distribution of RTTs observed. What rough shape is the distribution?

ans: The distribution graph is shown in lab4/ directory with name "qn8.png".The distribution is a mountain shape. High at middle RTT, and low at high and low RTT.

question 9: make a graph of the correlation between “RTT of ping #N” and “RTT of ping #N+1”.The x-axis should be the number of milliseconds from the first RTT, and the y-axis should be the number of milliseconds from the second RTT. How correlated is the RTT over time?





































