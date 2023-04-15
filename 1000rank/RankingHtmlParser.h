#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <lexbor/html/parser.h>
#include <lexbor/html/interfaces/document.h>
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/dom/interfaces/node.h>
#include <lexbor/dom/interfaces/comment.h>

using namespace std;

class RankingHtmlParser
{
public:
	RankingHtmlParser(const string& filename) : filename(filename), rank(1) {
		parse_html_file();
	}

	const unordered_map<string, int>& get_id_to_rank_map() const {
		return id_to_rank_map;
	}

private:
	string filename;
	unordered_map<string, int> id_to_rank_map;
	int rank;

	void parse_html_file() {
		// Read the HTML file
		ifstream file(filename);
		if (!file.is_open()) {
			cout << "Failed to open " << filename << endl;
			exit(1);
		}
		stringstream buffer;
		buffer << file.rdbuf();
		string html_content = buffer.str();

		// Parse the HTML content
		lxb_html_document_t* document = parse_html_content(html_content.c_str(), html_content.size());

		// Retrieve the root element
		lxb_dom_node_t* root = lxb_dom_interface_node(document->body);

		// Call the store_ranks function to store the ranks
		store_ranks(root);

		// Destroy the HTML document
		lxb_html_document_destroy(document);
	}

	lxb_html_document_t* parse_html_content(const char* html_content, size_t size)
	{
		lxb_status_t status;
		lxb_html_parser_t* parser = lxb_html_parser_create();
		lxb_html_document_t* document = lxb_html_document_create();

		status = lxb_html_parser_init(parser);
		if (status != LXB_STATUS_OK) {
			cerr << "Failed to initialize the parser" << endl;
			exit(EXIT_FAILURE);
		}

		document = lxb_html_parse(parser, (const lxb_char_t*)html_content, size);
		if (document == nullptr) {
			cerr << "Failed to parse the HTML content" << endl;
			exit(EXIT_FAILURE);
		}

		lxb_html_parser_destroy(parser);
		return document;
	}

	void store_ranks(lxb_dom_node_t* node) {
		while (node != nullptr) {
			if (node->type == LXB_DOM_NODE_TYPE_COMMENT) {
				size_t len; 
				lxb_char_t* text = lxb_dom_node_text_content(node, &len);
				string comment_text = string((char*)text, len);
				comment_text = comment_text.substr(1, comment_text.size() - 2);
				id_to_rank_map[comment_text] = rank;
				rank++;
			}

			if (node->first_child != nullptr) {
				store_ranks(node->first_child);
			}

			node = node->next;
		}
	}
};