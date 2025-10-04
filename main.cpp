#include <iostream>
#include <vector>
#include <string>
#include <queue> //для priority_queue
#include <cmath>

struct Node{
    std::string key;
    double prob;
    Node* left;
    Node* right;

     //конструктор листов
    Node(std::string k, double p) : key(k), prob(p), left(nullptr), right(nullptr) {}
    //конструктор родителей
    Node(Node* l, Node* r) : key(""), prob(l->prob + r->prob), left(l), right(r) {}
};

struct compareNodes {
    bool operator()(Node* a, Node* b) {
        return a->prob > b->prob;
    }
};

void codeGenerator (Node* node, std::string code, std::vector<std::string>& codes, std::vector<std::string>& symbols){
    if (node->left == nullptr && node->right == nullptr){ //проверка на лист
        for (int i = 0; i < symbols.size(); i++){
            if (node -> key == symbols[i]){ //если ключ текущего узла равен символу
                codes[i] = code; 
                return;
            }
        }
        return;
    }
    codeGenerator(node->left, code + "0", codes, symbols);
    codeGenerator(node->right, code + "1", codes, symbols);
}

int main(){
    std::cout << "\n\n=== ЗАДАНИЕ 1 ===\n";
    
    std::vector<std::string> symbols {"z1","z2","z3","z4","z5","z6","z7","z8"};
    std::vector<double> probs {0.26, 0.19, 0.14, 0.11, 0.10, 0.08, 0.07, 0.05};
    std::vector<std::string> codes {"", "", "", "", "", "", "", ""};

    //создание приоритетной очереди (вектора, который будет содержать указатели)
    std::priority_queue<Node*, std::vector<Node*>, compareNodes> pq;

    for (int i = 0; i < symbols.size(); i++) {
        pq.push(new Node(symbols[i], probs[i]));
    }

    while (pq.size() > 1) {
        Node* left = pq.top();
        pq.pop();
        Node* right = pq.top();
        pq.pop();
        
        Node* parent = new Node(left, right);
        pq.push(parent);
    }

    Node* root = pq.top();

    codeGenerator(root, "", codes, symbols);

    for (int i = 0; i < symbols.size(); i++) {
        std::cout << symbols[i] << "  p = " << probs[i] << ": " << codes[i] << std::endl;
    }
    
    double entropy = 0.0;
    double avg_length = 0.0;
    
    for (int i = 0; i < symbols.size(); i++) {
        if (probs[i] > 0) {
            entropy -= probs[i] * log2(probs[i]);
        }
        avg_length += probs[i] * codes[i].length();
    }
    
    double redundancy = (1-entropy/log2(8))*100; 

    std::cout << "\nЭнтропия (H): " << entropy << " bit/symb\n";
    std::cout << "Средняя длина (L): " << avg_length << " bit/symb\n";
    std::cout << "Избыточность (R = 1 - H/H(max)): " << redundancy << " bit/symb\n";

    //ДЛЯ ЗАДАНИЯ 2
    std::cout << "\n\n=== ЗАДАНИЕ 2 ===\n";
    
    std::vector<std::string> symbols2 {"z1", "z2"};
    std::vector<double> probs2 {0.9, 0.1};
    
    std::vector<std::string> comb_symbols;
    std::vector<double> comb_probs;
    std::vector<std::string> comb_codes;
    
    //генератор комбинаций
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                std::string comb = symbols2[i] + symbols2[j] + symbols2[k];
                double prob = probs2[i] * probs2[j] * probs2[k];
                
                comb_symbols.push_back(comb);
                comb_probs.push_back(prob);
                comb_codes.push_back("");
                
            }
        }
    }
    
    //коды Хаффмана для комбинаций
    std::priority_queue<Node*, std::vector<Node*>, compareNodes> pq2;
    
    for (int i = 0; i < comb_symbols.size(); i++) {
        pq2.push(new Node(comb_symbols[i], comb_probs[i]));
    }
    
    while (pq2.size() > 1) {
        Node* left = pq2.top();
        pq2.pop();
        Node* right = pq2.top();
        pq2.pop();
        
        Node* parent = new Node(left, right);
        pq2.push(parent);
    }
    
    Node* root2 = pq2.top();
    codeGenerator(root2, "", comb_codes, comb_symbols);

    for (int i = 0; i < comb_symbols.size(); i++) {
    std::cout << comb_symbols[i] << "  p = " << comb_probs[i] << ": " << comb_codes[i] << std::endl;
    }

    double comb_avg_length = 0.0;
    
    for (int i = 0; i < comb_symbols.size(); i++) {
        comb_avg_length += comb_probs[i] * comb_codes[i].length();
    }
    
    std::cout << "\nСредняя длина (L): " << comb_avg_length << " bit/block\n";

    //ДЛЯ ЗАДАНИЯ 3
    std::cout << "\n\n=== ЗАДАНИЕ 3 ===\n";

    std::vector<std::string> symbols3 {"T", "O", "K"};
    std::vector<double> probs3 {0.3, 0.3, 0.4};

    std::vector<std::string> block_symbols;
    std::vector<double> block_probs;
    std::vector<std::string> block_codes;

    //генератор пар по 2 символа
    for (int i = 0; i < symbols3.size(); i++) {
        for (int j = 0; j < symbols3.size(); j++) {
            std::string block = symbols3[i] + symbols3[j];
            double prob = probs3[i] * probs3[j];
            
            block_symbols.push_back(block);
            block_probs.push_back(prob);
            block_codes.push_back("");
        }
    }

    std::priority_queue<Node*, std::vector<Node*>, compareNodes> pq3;

    for (int i = 0; i < block_symbols.size(); i++) {
        pq3.push(new Node(block_symbols[i], block_probs[i]));
    }
    
    while (pq3.size() > 1) {
        Node* left = pq3.top();
        pq3.pop();
        Node* right = pq3.top();
        pq3.pop();
        
        Node* parent = new Node(left, right);
        pq3.push(parent);
    }
    
    Node* root3 = pq3.top();
    codeGenerator(root3, "", block_codes, block_symbols);

    for (int i = 0; i < block_symbols.size(); i++) {
        std::cout << block_symbols[i] << "  p = " << block_probs[i] << ": " << block_codes[i] << "\n";
    }

    double block_avg_length = 0.0;
    
    for (int i = 0; i < block_symbols.size(); i++) {
        block_avg_length += block_probs[i] * block_codes[i].length();
    }

    std::cout << "\nСредняя длина (L): " << block_avg_length << " bit/block\n";

    return 0;
}