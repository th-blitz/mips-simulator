

trace_file = open('trace.txt')
trace_out_file = open('trace.txt.out')
trace_expected = open('expected_results/trace.txt.out.ans.txt')

i = 0
for file1, file2 in zip(trace_out_file, trace_expected):
    if (file1 != file2):
        print(i + 1, file1, file2)
    i += 1

trace_lines = []
for line in trace_file:
    trace_lines.append(line)

# i = 0
# for line in trace_lines:
#     if (int(line[2 :-1], 0) & 0xfffffff8) == (0xfffffff8 & 0x80481bc):
#         print(i + 1, line)
#     i += 1
