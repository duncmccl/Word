#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <string>
#include <list>
#include <filesystem>
#include <chrono>
#include <cmath>



/*

Project Idea:

Relavance of a keyword and document is inversly proportional to the standard deviation of the keyword in the document.

Normalized index = keyword index / number of words in document

Normalized Mean = Sum( normalized index ) / number of keywords in document

Normalized Standard deviation = Sqrt( Sum( ( normalized index - mean ) ^ 2 ) / number of keywords in document )



The idea is that given documents and a keyword that appears the same number 
of times in each document: if the keyword is spread across a document evenly, 
it would be more relavant than a document which has the keyword mentioned
only in a small area.







Class notes:

Precision = Relevant Retrieved / Retrieved

Recall = Relevant Retrieved / Relevant

F-Measure = ((B*B + 1) * P * R) / (B*B*P + R)
F1 -> B = 1

F1 = ( 2 * P * R ) / ( P + R )



Mean Average precision (MAP)

Sum[ AvgPrec(N) / N, {N, 1, ( number of retrieved documents )} ] / ( number of relavant documents )

A: +   -     +     -   -
   1 + 0 + (2/3) + 0 + 0

MAP = (1 + (2/3) ) / 5 = 1/3



Q1: 3 relavant
A: + + - - -	( ( 1 / 1 ) + ( 2 / 2 ) + ( 0 / 3 ) + ( 0 / 4 ) + ( 0 / 5 ) ) / 3 = 0.6666
B: - - + - -	( ( 0 / 1 ) + ( 0 / 2 ) + ( 1 / 3 ) + ( 0 / 4 ) + ( 0 / 5 ) ) / 3 = 0.1111



Q2: 5 relavant
A: - - + + +	( ( 0 / 1 ) + ( 0 / 2 ) + ( 1 / 3 ) + ( 2 / 4 ) + ( 3 / 5 ) ) / 5 = 0.2866
B: + + - - +	( ( 1 / 1 ) + ( 2 / 2 ) + ( 0 / 3 ) + ( 0 / 4 ) + ( 3 / 5 ) ) / 5 = 0.5200

MAP(A) = ( 0.6666 + 0.2866 ) / 2 = 0.4766
MAP(B) = ( 0.1111 + 0.5200 ) / 2 = 0.3156



Normalized Discounted Cumulative Gain (nDCG)

Cumulative Gain at rank N
	CG = r1 + r2 + ...rN

Discounted Cumulative Gain at rank N
	DCG = ( r1 ) + ( r2 / log2(2) ) + ( r3 / log2(3) ) + ... ( rN / log2(N) )

Normalized Discounted Cumulative Gain
	nDCG = DCG / IdealDCG
	
	IdealDCG = DCG on set of Ideal Rankings

*/



double jaro( const std::string s1, const std::string s2 ) {
	const uint l1 = s1.length(), l2 = s2.length();
	if (l1 == 0) 
		return l2 == 0 ? 1.0 : 0.0;
	
	const uint match_distance = std::max(l1, l2) / 2 - 1;
	
	bool s1_matches[l1];
	bool s2_matches[l2];
	
	std::fill(s1_matches, s1_matches + l1, false);
	std::fill(s2_matches, s2_matches + l2, false);
	
	uint matches = 0;
	
	for ( uint i = 0; i < l1; i++ ) {
		
		const int end = std::min( i + match_distance + 1, l2);
		
		for(int k = std::max(0u, i - match_distance); k < end; k++ ) {
			
			if (!s2_matches[k] && s1[i] == s2[k]) {
				
				s1_matches[i] = true;
				s2_matches[k] = true;
				matches++;
				break;
				
			}
			
		}
		
	}
	
	if ( matches == 0 ) 
		return 0.0;
	
	double t = 0.0;
	
	uint k = 0;
	
	for( uint i = 0; i < l1; i++ ) {
		
		if(s1_matches[i]) {
			while(!s2_matches[k]) k++;
			if (s1[i] != s2[k]) t += 0.5;
			k++;
		}
		
	}
	
	const double m = matches;
	
	return ( m / l1 + m / l2 + ( m - t ) / m ) / 3.0;
	
}


double string_similarity( const std::string str_A, const std::string str_B) {
	
	
	std::size_t sml_len = str_A.size();
	std::size_t big_len = str_B.size();
	bool order = sml_len <= big_len;
	
	if (!order) {
		std::swap(sml_len, big_len);
	}
	
	
	std::size_t best_dist = SIZE_MAX;
	std::size_t best_count = 0;
	
	for( std::size_t offset = 0; offset <= (big_len - sml_len); offset++ ) {
		
		std::string tmp_A = str_A;
		std::string tmp_B = str_B;
		
		if (!order) {
			std::swap(tmp_A, tmp_B);
		}
		
		std::size_t similar_count = 0;
		std::size_t dist_acc = 0;
		
		for( std::size_t i = 0; i < sml_len; i++ ) {
			
			bool found = 0;
			std::size_t best_dist = SIZE_MAX;
			std::size_t best_indx = 0;
			
			for( std::size_t j = 0; j < big_len; j++ ) {
				
				if ( tmp_A[i] == tmp_B[(j + offset) % big_len] ) {
					
					std::size_t dist = (i > j ? i - j : j - i);
					dist *= dist;
					
					if (!found || dist < best_dist) {
						
						best_dist = dist;
						best_indx = j;
						found = 1;
						
					}
					
				}
				
			}
			
			
			if (found) {
				
				tmp_A[i] = 0;
				tmp_B[(best_indx + offset) % big_len] = 0;
				
				similar_count++;
				dist_acc += best_dist;
			}
			
		}
		
		if (similar_count > best_count || (similar_count == best_count && dist_acc < best_dist)) {
			
			best_count = similar_count;
			best_dist = dist_acc;
			
		}
		
	}
	
	// TODO : Tweak constants and such.
	return ((double)best_count / ((double)big_len + sqrt(best_dist)));
	
}




double standatd_deviation(std::list< std::tuple< std::size_t, std::size_t, std::size_t > > keyword_indices, std::size_t doc_length) {
	
	std::list< double > normalized_indices;
	for( const auto& [ row, col, index ] : keyword_indices ) {
		normalized_indices.push_back( (double) index / (double) doc_length);
	}
	
	
	double mean = 0.0;
	
	for( const auto& n_index : normalized_indices ) {
		mean += n_index;
	}
	mean /= normalized_indices.size();
	
	
	double standard_deviation = 0;
	for( const auto& n_index : normalized_indices ) {
		standard_deviation += pow(n_index - mean, 2.0);
	}
	standard_deviation = sqrt(standard_deviation / normalized_indices.size());
	
	return standard_deviation;
}


void tabulate( std::string file_path, 
			   std::map< std::size_t, std::map< std::size_t, std::list< std::tuple< std::size_t, std::size_t, std::size_t > > > >& dictionary, 
			   std::map< std::size_t, std::tuple< std::size_t, std::size_t > >& stats, 
			   std::map< std::size_t, std::string >& resolver) {
	
	if(!std::filesystem::is_regular_file(file_path)) return;
	
	std::ifstream file;
	file.open(file_path.c_str());
	
	std::cout << "\t" << file_path << std::endl;
	
	std::size_t filename_hash = std::hash< std::string >{}(file_path);
	resolver[filename_hash] = file_path;
	
	std::string line;
	std::string word;
	
	std::size_t word_count = 0;
	std::size_t unique_count = 0;
	
	std::size_t row = 0;
	std::size_t col = 0;
	
	while(std::getline(file, line)) {
		
		std::istringstream linestream(line);
		col = 0;
		
		while(linestream >> word) {

			// remove any non alphabet characters
			word.erase(std::remove_if(word.begin(), word.end(), [](const auto& c) -> bool { return (c < 'A' || (c > 'Z' && c < 'a') || c > 'z'); } ), word.end());

			// To lowercase
			std::transform(word.begin(), word.end(), word.begin(), [](const auto& c) { return std::tolower(c); } );

			// If anything remains
			if (word.size() > 0) {
				
				std::size_t word_hash = std::hash< std::string >{}(word);
				resolver[word_hash] = word;
				
				// If word has not been in file before
				if (dictionary[word_hash][filename_hash].size() == 0) unique_count++;
				
				// Record occurance of the word in dictionary
				dictionary[word_hash][filename_hash].push_back(std::make_tuple(row, col, word_count));
				
			}
			
			word_count++;
			col++;
		}
		
		row++;
	}
	
	file.close();
	
	stats[filename_hash] = std::make_tuple(word_count, unique_count);
	
}


int main() {
	
	// Create dictionary
	// Map[ Word Hash : Map[ Filename Hash : List[ Tuple[ row, col, index ] ] ] ]
	std::map< std::size_t, std::map< std::size_t, std::list< std::tuple< std::size_t, std::size_t, std::size_t > > > > dictionary;
	
	
	
	// Create Hashmap for file statistics
	// HashMap[ Filename Hash : Tuple[ Number of words in doc, Number of unique words in Doc ] ]
	std::map< std::size_t, std::tuple< std::size_t, std::size_t > > stats;
	
	
	
	// Create Hashmap for hash to string resolver
	// HashMap[ Word or Filename Hash : String ]
	std::map< std::size_t, std::string > resolver;
	
	
	
	// Tabulate all words in all files in dir 'toread'
	std::string read_path = "./toread/";
	
	std::cout << "Tabulating..." << std::endl;
	
	for( const auto& document : std::filesystem::recursive_directory_iterator(read_path))
        tabulate(document.path(), dictionary, stats, resolver);
	
	std::cout << std::endl;
	
	
	
	std::size_t average_document_length = 0.0;
	for( const auto& [ filename_hash, file_stats ] : stats ) {
		average_document_length += std::get<0>(file_stats);
	}
	average_document_length /= stats.size();
	
	
	std::cout << "CREATING DICTIONARY" << std::endl << std::endl;
	
	std::ofstream dict_file;
	dict_file.open("dictionary.csv");
	
	for (const auto & [word_hash, dictionary_entry] : dictionary) {
		
		//std::cout << "Word: " << resolver[word_hash] << std::endl;
		//std::cout << "Hash: " << word_hash << std::endl;
		//std::cout << "File Count: " << dictionary_entry.size() << std::endl;
		
		
		dict_file << word_hash;
		
		for (const auto& [filename_hash, indices] : dictionary_entry) {
			
			//std::cout << "| File name: " << resolver[filename_hash] << std::endl;
			//std::cout << "| File hash: " << filename_hash << std::endl;
			//std::cout << "| Count: " << indices.size() << std::endl;
			//std::cout << "|        Row       Col     Index" << std::endl;
			
			//for( const auto& [row, col, index] : indices) {
				
				//std::cout << "| " << std::setw(10) << row << std::setw(10) << col << std::setw(10) << index << std::endl;
				
			//}
			
			//std::cout << "|" << std::endl;
			
			
			const std::size_t num_documents_total = stats.size();
			const std::size_t num_documents_with_keyword = dictionary_entry.size();
			const std::size_t num_keywords_in_document = indices.size();
			const std::size_t num_words_in_document = std::get<0>(stats[filename_hash]);
			
			// TODO : Find ideal value for b
			const double b = 0.5;
			
			double relavance_factor = 
				((1.0 + log(1.0 + log((double)num_keywords_in_document))) / 				// TF
				((1.0 - b) + (b * (num_words_in_document / average_document_length)))) * 	// Length Normalization
				log((num_documents_total + 1.0) / num_documents_with_keyword); 				// IDF
			
			
			dict_file << ", " << filename_hash << ", " << relavance_factor;
			
			
		}
		
		//std::cout << std::endl;
		
		dict_file << std::endl;
	}
	
	dict_file.close();
	
	
	std::cout << "CREATING RESOLVER" << std::endl << std::endl;
	
	std::ofstream resv_file;
	resv_file.open("resolve.csv");
	
	for( const auto& [ hash, str ] : resolver ) {
		
		resv_file << hash << ", " << str << std::endl;
		
	}
	
	resv_file.close();
	
	/*
	std::cout << "STATS" << std::endl << std::endl;
	for( const auto& [ filename_hash, file_stats ] : stats ) {
		
		std::cout << "File Name: " << resolver[filename_hash] << std::endl;
		
		auto& [ word_count, unique_count ] = file_stats;
		
		std::cout << " Total Word Count: " << std::setw(10) << word_count << std::endl;
		std::cout << "Unique Word Count: " << std::setw(10) << unique_count << std::endl;
		std::cout << std::endl;
		
	}
	*/
	
	
	/*
	std::string query = "abcense acceptible accidentaly acommodate acheive aknowledge aquire";
	std::istringstream querystream(query);
	std::string word;
	while( querystream >> word ) {
		
		for( const auto& [ key, string ] : resolver ) {
			
			double cmp1 = string_similarity(word, string);
			double cmp2 = jaro(word, string);
			
			if (cmp1 > 0.75 || cmp2 > 0.75) {
				
				std::cout << std::setw(15) << word << " vs ";
				std::cout << std::setw(15) << string << " : ";
				std::cout << std::setw(15) << cmp1;
				std::cout << std::setw(15) << cmp2;
				std::cout << std::endl;
				
			}
			
		}
		
		std::cout << std::endl;
		
	}
	
	
	double cmp1;
	double cmp2;
	
	cmp1 = string_similarity("apple", "orange");
	cmp2 = jaro("apple", "orange");
	std::cout << std::setw(30) << "apple vs orange : ";
	std::cout << std::setw(10) << cmp1 << " | ";
	std::cout << std::setw(10) << cmp2 << std::endl;
	
	cmp1 = string_similarity("unreliable", "reliable");
	cmp2 = jaro("unreliable", "reliable");
	std::cout << std::setw(30) << "unreliable vs reliable : ";
	std::cout << std::setw(10) << cmp1 << " | ";
	std::cout << std::setw(10) << cmp2 << std::endl;
	
	cmp1 = string_similarity("accomodate", "acommodate");
	cmp2 = jaro("accomodate", "acommodate");
	std::cout << std::setw(30) << "accomodate vs acommodate : ";
	std::cout << std::setw(10) << cmp1 << " | ";
	std::cout << std::setw(10) << cmp2 << std::endl;
	
	cmp1 = string_similarity("hmmhmmhmm", "mmmmmmhhh");
	cmp2 = jaro("hmmhmmhmm", "mmmmmmhhh");
	std::cout << std::setw(30) << "hmmhmmhmm vs mmmmmmhhh : ";
	std::cout << std::setw(10) << cmp1 << " | ";
	std::cout << std::setw(10) << cmp2 << std::endl;
	*/
	
	
	return 0;
}