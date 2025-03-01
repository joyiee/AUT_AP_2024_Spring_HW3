#include "Trie.h"
#include <algorithm>
#include <stack>
#include <string>
#include <queue>
#include <set>
Trie::Node::Node(char data, bool is_finished):parent(nullptr),children{},data(data),is_finished(is_finished){}

Trie::Node::~Node()
{
    for(auto child : children)
    {
        delete child;
        child = nullptr;
    }
}

Trie::Trie():root(new Node()){}

Trie::Trie(const Trie &other):root(new Node())
{
    if(other.root)
        copyNodes(root, other.root);
}

Trie::Trie(Trie &&other):root(other.root)
{
    other.root = nullptr;
}

Trie::Trie(std::initializer_list<std::string> list):root(new Node())
{
    for(const auto&word : list)
        insert(word);
}

Trie::~Trie()
{
    delete root;
}

Trie& Trie::operator=(const Trie& other)
{
    if(*this == other) return *this;

    delete root;
    root = new Node();
    if(other.root)
        copyNodes(root, other.root);
    
    return *this;
}

Trie& Trie::operator=(Trie&& other)
{
    if(*this == other) return *this;

    delete root;
    root = other.root;
    other.root = nullptr;

    return *this;
}

void Trie::insert(const std::string& str)
{
    Node* current = root;
    for(char ch:str)
    {
        int index = ch - 'a';
        if(!current->children[index])
        {
            current->children[index] = new Node(ch);
            current->children[index]->parent = current;
        }
        current = current->children[index];
    }
    current->is_finished = true;
}

bool Trie::search(const std::string& query) const
{
    Node* current = root;
    for(char ch: query)
    {
        int index = ch - 'a';
        if(!current->children[index])
            return false;
        current = current->children[index];
    }
    return current->is_finished;
}

bool Trie::startsWith(const std::string& prefix) const
{
    Node* current = root;
    for(char ch: prefix)
    {
        int index = ch - 'a';
        if(!current->children[index])
            return false;
        current = current->children[index];
    }
    return true;
}


void Trie::copyNodes(Node* thisNode, Node* otherNode) {
    thisNode->is_finished = otherNode->is_finished;
    thisNode->data = otherNode->data;
    for (int i = 0; i < 26; ++i) {
        if (otherNode->children[i]) {
            thisNode->children[i] = new Node();
            thisNode->children[i]->parent = thisNode;
            copyNodes(thisNode->children[i], otherNode->children[i]);
        }
    }
}

void Trie::remove(const std::string& str)
{
    Node* current = root;
    for(char ch: str)
    {
        int index = ch - 'a';
        if(!current->children[index])
            return;
        current = current->children[index];
    }
    if (!current->is_finished)
        return;
    current->is_finished = false;

    for(int i = str.size()-1; i >= 0; i--)
    {
        Node *parent = current->parent;
        if(std::all_of(current->children.begin(),current->children.end(),[](Node* child){
            return child == nullptr;
        }))
        {
            delete current;
            parent->children[str[i] - 'a'] = nullptr;
        }
        current = parent;
    }
}

void Trie::bfs(std::function<void(Node*&)> func)
{
    if(!root) return;
    std::queue<Node*> myqueue;
    myqueue.push(root);
    while(!myqueue.empty())
    {
        Node *tmp = myqueue.front();
        myqueue.pop();

        func(tmp);

        for(auto child : tmp->children)
        {
            if(child)
                myqueue.push(child);
        }
    }
}

void Trie::dfs(std::function<void(Node*&)> func)
{
    if(!root) return;
    std::stack<Node*> mystack;
    mystack.push(root);
    while(!mystack.empty())
    {
        Node* tmp = mystack.top();
        mystack.pop();
        func(tmp);

        for (int i = 25; i >= 0; --i) {
            if (tmp->children[i]) {
                mystack.push(tmp->children[i]);
            }
        }
    }
}

std::ostream& operator<<(std::ostream& os, const Trie& trie)
{
    std::function<void(Trie::Node*, std::string)> collectWords;
    collectWords = [&](Trie::Node* node, std::string currentWord)
    {
        if(!node) return;
        if(node->is_finished)
            os << currentWord <<" ";
        
        for(int i = 0; i < 26; i++)
        {
            if(node->children[i])
            {
                collectWords(node->children[i], currentWord + char('a'+i));
            }
        }
    };
    
    collectWords(trie.root, "");
    return os;
}

std::istream& operator>>(std::istream& is, Trie& trie)
{
    std::string word;
    while(is >> word)
    {
        trie.insert(word);
    }
    return is;
}

Trie Trie::operator+(const Trie& other) const
{
    Trie result = *this;
    std::function<void(Node*, std::string)> traverse;
    traverse = [&](Node* node, std::string currentWord)
    {
        if(!node) return;
        if(node->is_finished)
            result.insert(currentWord);
        for(int i = 0; i<26; i++)
        {
            if(node->children[i])
            {
                traverse(node->children[i], currentWord + char('a'+i));
    
            }
        }
    };
    traverse(other.root, "");
    return result;
}

Trie& Trie::operator+=(const Trie& other)
{
    std::function<void(Node*, std::string)> traverse;
    traverse = [&](Node* node, std::string currentWord)
    {
        if(!node) return;
        if(node->is_finished)
            this->insert(currentWord);
        for(int i = 0; i<26; i++)
        {
            if(node->children[i])
            {
                traverse(node->children[i], currentWord + char('a'+i));
    
            }
        }
    };
    traverse(other.root, "");
    return *this;
}

Trie Trie::operator-(const Trie& other) const
{
    Trie result = *this;
    std::function<void(Node*, std::string)> traverse;
    traverse = [&](Node* node, std::string currentWord)
    {
        if(!node) return;
        if(node->is_finished)
            result.remove(currentWord);
        for(int i = 0; i<26; i++)
        {
            if(node->children[i])
            {
                traverse(node->children[i], currentWord + char('a'+i));
    
            }
        }
    };
    traverse(other.root, "");
    return result;
}

Trie& Trie::operator-=(const Trie& other)
{
    std::function<void(Node*, std::string)> traverse;
    traverse = [&](Node* node, std::string currentWord)
    {
        if(!node) return;
        if(node->is_finished)
            this->remove(currentWord);
        for(int i = 0; i<26; i++)
        {
            if(node->children[i])
            {
                traverse(node->children[i], currentWord + char('a'+i));
    
            }
        }
    };
    traverse(other.root, "");
    return *this;
}

bool Trie::operator()(const std::string& query) const
{
    return this->search(query);
}

bool Trie::operator==(const Trie& other) const
{
    std::set<std::string> set1, set2;
    std::function<void(Trie::Node*, std::string, std::set<std::string>&)> collectWords;
    collectWords = [&](Trie::Node* node, std::string currentWord, std::set<std::string>& words)
    {
        if(!node) return;
        if(node->is_finished)
            words.insert(currentWord);
        for(int i = 0; i < 26; i++)
        {
            if(node->children[i])
            {
                collectWords(node->children[i], currentWord + char('a'+i), words);
            }
        }
    };

    collectWords(this->root, "", set1);
    collectWords(other.root, "", set2);

    return set1 == set2;

}

bool Trie::operator!=(const Trie& other) const
{
    return !(*this==other);
}

