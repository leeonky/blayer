#!/bin/awk -f
function output_json(value, key) {
	cmd = "jq -R 'rtrimstr(\"\n\")' >> " result_file
	print "\""key"\": " >> result_file
	print value | cmd
	close(cmd)
}

function output_array_json(value, key) {
	cmd = "jq -R -s 'rtrimstr(\"\n\") | rtrimstr(\"\n\") | rtrimstr(\"\n\")' >> " result_file
	print "\""key"\": " >> result_file
	print value | cmd
	close(cmd)
}

function out_put_suite(new_suite) {
	if (is_in_suit()) {
		for(i=0; i<=case_count; i++) {
			print "{" >> result_file
			output_json(suite_name, "suite")
			print "," >> result_file
			output_json(case_names[i], "case")
			print "," >> result_file
			output_json(case_result[i], "result")
			print "," >> result_file
			output_array_json(case_output[i], "output")
			print "}" >> result_file
		}
	}

	suite_name = new_suite
	case_count = -1
}

function is_in_suit() {
	return "" != suite_name
}

function change_to_new_case(new_case, result) {
	case_count++
	case_names[case_count] = new_case
	case_result[case_count] = result
	case_output[case_count] = ""
	if(result == "passed") {
		printf "."
	} else {
		printf "F"
	}
}

function is_in_case() {
	return case_count >= 0
}

function appent_test_result(line) {
	case_output[case_count] = case_output[case_count] line "\n"
}

BEGIN {
	suite_name = ""
	case_count = -1
	skip = 0
	result_file = "/tmp/res.json"
	system("rm " result_file)
	not_test = 1
}
{
	if (match($0, /^===========/)) {
		not_test = 0
	}
	if(not_test) {
		print $0
	} else {
		if (match($0, /Run Summary:/)) {
			skip = 1
		}
		if (match($0, /^Suite: (.*)/, suite)) {
			skip = 0
		}
		if (!skip) {
			if (match($0, /^Suite: (.*)/, suite)) {
				out_put_suite(suite[1])
			} else if (is_in_suit() && match($0, /^  Test: (.*) \.\.\.(.*)$/, cases)) {
				change_to_new_case(cases[1], cases[2])
			} else if (is_in_case()) {
				appent_test_result($0)
			}
		}
	}
}
END {
	out_put_suite("")
}
