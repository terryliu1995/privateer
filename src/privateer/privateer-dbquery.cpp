// Library for the YSBL program Privateer (PRogramatic Identification of Various Anomalies Toothsome Entities Experience in Refinement)
// Licence: LGPL (https://www.gnu.org/licenses/lgpl.html)
//
// 2013-2018 Haroldas Bagdonas & Kevin Cowtan & Jon Agirre
// York Structural Biology Laboratory
// The University of York
// mailto: hb1115@york.ac.uk
// mailto: jon.agirre@york.ac.uk
// mailto: kevin.cowtan@york.ac.uk

// Most likely need to create a method in MGlycan that is responsible for eliminating leaf nodes in MGlycan class
// After that is created, the rest should be straighforward.

#include "privateer-dbquery.h"

void output_dbquery(nlohmann::json &jsonObject, clipper::String glycanWURCS, clipper::MGlycan &currentGlycan, std::vector<std::pair<std::pair<clipper::MGlycan, std::vector<int>>,float>>& finalGlycanPermutationContainer, bool glucose_only)
{
    int valueLocation;
    valueLocation = privateer::util::find_index_of_value(jsonObject, "Sequence", glycanWURCS);
    if (valueLocation != -1 && jsonObject[valueLocation]["glyconnect"] != "NotFound") print_output_from_database(jsonObject, valueLocation, currentGlycan);
    else
    {
        if (valueLocation != -1 && currentGlycan.number_of_nodes() > 1)
        {
            print_output_from_database(jsonObject, valueLocation, currentGlycan);
            std::cout << "\nWARNING: Unable to find a matching GlyTouCanID for WURCS sequence from this Glycan sequence! Attempting to find the closest matches by carrying out permutations" << std::endl;

            std::vector<std::pair<clipper::MGlycan, std::vector<int>>> alternativeGlycans;
            alternativeGlycans = generate_closest_matches(currentGlycan, jsonObject, glucose_only);
        
            if (!alternativeGlycans.empty()) push_data_to_final_permutation_container(jsonObject, currentGlycan, alternativeGlycans, finalGlycanPermutationContainer);    
            else std::cout << "ERROR: Unable to generate permutations that would be found in GlyConnect database!" << std::endl;
        }
        else if (valueLocation != -1 && currentGlycan.number_of_nodes() <= 1)
        {
            print_output_from_database(jsonObject, valueLocation, currentGlycan);
            std::cout << "ERROR: Glycan is too short for permutations, therefore unable to find the closest match on GlyConnect." << std::endl;
        }
        else
        {
            if ( currentGlycan.number_of_nodes() > 1)
            {
                std::vector<std::pair<clipper::MGlycan, std::vector<int>>> alternativeGlycans;
                alternativeGlycans = generate_closest_matches(currentGlycan, jsonObject, glucose_only);
            
                if (!alternativeGlycans.empty()) push_data_to_final_permutation_container(jsonObject, currentGlycan, alternativeGlycans, finalGlycanPermutationContainer);    
                else std::cout << "ERROR: Unable to generate permutations that would be found in GlyConnect database!" << std::endl;
            }
            else std::cout << "ERROR: Glycan is too short for permutations, therefore unable to generate alternative GlyTouCan and GlyConnect IDs!" << std::endl;
         }
    }
}


void push_data_to_final_permutation_container(nlohmann::json &jsonObject, clipper::MGlycan &currentGlycan, std::vector<std::pair<clipper::MGlycan, std::vector<int>>>& alternativeGlycans, std::vector<std::pair<std::pair<clipper::MGlycan, std::vector<int>>,float>>& finalGlycanPermutationContainer)
{
    for (int j = 0; j < alternativeGlycans.size(); j++)
        {
            int originalGlycanLength = currentGlycan.number_of_nodes(),
                currentGlycanLength = alternativeGlycans[j].first.number_of_nodes();

            int anomerPermutations = alternativeGlycans[j].second[0],
                residuePermutations = alternativeGlycans[j].second[1],
                residueDeletions = alternativeGlycans[j].second[2];
            
            float finalScore, maxPermutationScore, currentPermutationScore;

            maxPermutationScore = ( ( (currentGlycanLength * 5) + (currentGlycanLength * 25) + ((originalGlycanLength - 1) * 100) ) / originalGlycanLength );
            currentPermutationScore = ( ( (anomerPermutations * 5) + (residuePermutations * 25) + (residueDeletions * 100) ) / originalGlycanLength );
            finalScore = (currentPermutationScore / maxPermutationScore) * 100;

            auto tempObject = std::make_pair(alternativeGlycans[j], finalScore);
            finalGlycanPermutationContainer.push_back(tempObject);

            clipper::String temporaryWURCS = alternativeGlycans[j].first.generate_wurcs();
            int valueLocation = privateer::util::find_index_of_value(jsonObject, "Sequence", temporaryWURCS);

            std::cout << "\tGenerated WURCS Sequence: " << temporaryWURCS << std::endl;
            std::cout << "\tAnomer Permutations = " << alternativeGlycans[j].second[0] << "\t\tResidue Permutations = " << alternativeGlycans[j].second[1] << "\tResidue Deletions = " << alternativeGlycans[j].second[2] << std::endl; 
            std::cout << std::fixed << std::setprecision(2) << "\tPermutation Score(out of 100): " << finalScore << std::endl;

            std::string glytoucanID;
            glytoucanID = jsonObject[valueLocation]["AccessionNumber"];
            if (glytoucanID.front() == '"' && glytoucanID.front() == '"')
            {
                glytoucanID.erase(0, 1);
                glytoucanID.pop_back();
            }
            std::cout << "\tGlyTouCan Accession ID: " << glytoucanID << std::endl;
            std::cout << "\tGlyConnect ID: " << jsonObject[valueLocation]["glyconnect"]["id"] << std::endl;

            std::cout << std::endl;
        }
}


void print_output_from_database(nlohmann::json& jsonObject, int valueLocation, clipper::MGlycan &currentGlycan)
{
    std::string glytoucanID;
    glytoucanID = jsonObject[valueLocation]["AccessionNumber"];
    if (glytoucanID.front() == '"' && glytoucanID.front() == '"')
    {
        glytoucanID.erase(0, 1);
        glytoucanID.pop_back();
    }
    std::cout << "\tManaged to find a matching GlyTouCan ID for WURCS sequence for this Glycan sequence!" << std::endl;
    std::cout << "\tGlyTouCan Accession ID: " << glytoucanID << std::endl;
    std::cout << "\tGlyTouCan link: "
              << "https://glytoucan.org/Structures/Glycans/" << glytoucanID << std::endl;

    if (jsonObject[valueLocation]["glyconnect"] != "NotFound")
    {
        if (!jsonObject[valueLocation]["glyconnect"]["comment"].is_null())
        {
            std::cout << "\t\tFound a GlyConnect entry for this GlyTouCan ID!" << std::endl;
            std::cout << "\t\tGlyConnect ID: " << jsonObject[valueLocation]["glyconnect"]["id"] << std::endl;
            std::cout << "\t\tGlycan Type: " << jsonObject[valueLocation]["glyconnect"]["type"] << std::endl;
            std::cout << "\t\tGlycan Core: " << jsonObject[valueLocation]["glyconnect"]["core"] << std::endl;
            std::cout << "\t\t" << jsonObject[valueLocation]["glyconnect"]["comment"] << std::endl;

            std::cout << std::endl
                      << "\t\tPrivateer checks: " << std::endl;
            std::cout << "\t\tGlycosylation type detected in the model: " << currentGlycan.get_type() << "\tGlycosylation type deposited on GlyConnect: " << jsonObject[valueLocation]["glyconnect"]["type"] << std::endl;
        }
        else
        {
            bool typeMatch = false;

            auto sourcesArray = jsonObject[valueLocation]["glyconnect"]["sources"];

            std::cout << "\t\tFound a GlyConnect entry for this GlyTouCan ID!" << std::endl;
            std::cout << "\t\tGlyConnect ID: " << jsonObject[valueLocation]["glyconnect"]["id"] << std::endl;
            std::cout << "\t\tGlycan Type: " << jsonObject[valueLocation]["glyconnect"]["type"] << std::endl;
            std::cout << "\t\tGlycan Core: " << jsonObject[valueLocation]["glyconnect"]["core"] << std::endl;
            std::cout << "\t\tGlycomics composition: " << jsonObject[valueLocation]["glyconnect"]["composition_string"] << std::endl;
            std::cout << "\t\tExpression system(s): " << std::endl;
            for (auto &element : sourcesArray)
            {
                std::cout << "\t\t\t\t\t" << element["species"] << std::endl;
            }
            std::cout << "\t\tTissue(s): " << std::endl;
            for (auto &element : sourcesArray)
            {
                std::cout << "\t\t\t\t\t" << element["system"] << std::endl;
            }
            std::cout << "\t\tProtein(s): " << std::endl;
            for (auto &element : sourcesArray)
            {
                std::cout << "\t\t\t\t\t" << element["protein"]["name"] << std::endl;
            }
            std::cout << "\t\tReviewed by GlyConnect: " << jsonObject[valueLocation]["glyconnect"]["reviewed"] << std::endl;
            std::cout << "\t\tGlyConnect link: "
                      << "https://glyconnect.expasy.org/browser/structures/" << jsonObject[valueLocation]["glyconnect"]["id"] << std::endl;

            std::cout << std::endl
                      << "\t\tPrivateer checks: " << std::endl;
            std::cout << "\t\tGlycosylation type detected in the model: " << currentGlycan.get_type() << "\tGlycosylation type deposited on GlyConnect: " << jsonObject[valueLocation]["glyconnect"]["type"] << std::endl;
        }
    }
    else
    {
        std::cout << "\t\tThis GlyTouCan ID is not deposited on GlyConnect." << std::endl;
    }
}

