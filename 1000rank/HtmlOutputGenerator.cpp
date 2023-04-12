#include "HtmlOutputGenerator.h"

void HtmlOutputGenerator::generate_html(const string& top_player_tag)
{
	// Load the template file
	ifstream template_file(template_file_path);
	stringstream template_stream;
	template_stream << template_file.rdbuf();
	string template_string = template_stream.str();
	// Create the context
	mstch::array players_data;
	players_data.push_back(mstch::map{ {"top_player_tag", top_player_tag} });
	// Render the template
	string rendered_html = mstch::render(template_string, mstch::map{ {"players", players_data} });
	// Write the rendered template to the output file
	ofstream output_file(output_file_path);
	output_file << rendered_html;
	output_file.close();
}