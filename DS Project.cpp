#include <iostream>

using namespace std;

const int NAME_LEN=32;
const int LINE_LEN=200;
const int MAX_STACK=100;
const int MAX_QUEUE=100;

int strLen(const char* s){
	int i=0;
	while(s[i]!='\0') i++;
	return i;
}

bool strEq(const char* a, const char* b){
	int i=0;
	while(a[i]!='\0' && b[i]!='\0'){
		if(a[i]!=b[i]) return false;
		i++;
	}
	return a[i]==b[i];
}

void strCopy(char* dst, const char* src, int maxLen){
	int i=0;
	while(src[i]!='\0' && i<maxLen-1){
		dst[i]=src[i];
		i++;
	}
	dst[i]='\0';
}

bool contains(const char* text, const char* pattern){
	int n=strLen(text), m=strLen(pattern);
	int i;
	for(i=0;i+m<=n;i++){
		int j=0;
		while(j<m && text[i+j]==pattern[j]) j++;
		if(j==m) return true;
	}
	return false;
}

const char* readWord(const char* p, char* out, int maxLen){
	while(*p==' ') p++;
	int k=0;
	while(*p!='\0' && *p!=' '){
		if(k<maxLen-1) out[k++]=*p;
		p++;
	}
	out[k]='\0';
	while(*p==' ') p++;
	return p;
}

struct Node{
	char name[NAME_LEN];
	bool isDir;
	char content[LINE_LEN];
	Node* parent;
	Node* firstChild;
	Node* nextSibling;
};

Node* root=NULL;
Node* cwd=NULL;

Node* stackArr[MAX_STACK];
int stackTop=0;

char queueArr[MAX_QUEUE][LINE_LEN];
int qFront=0;
int qRear=0;
int qCount=0;

bool batchMode=false;

Node* newNode(const char* name, bool isDir, Node* parent){
	Node* n=new Node;
	strCopy(n->name,name,NAME_LEN);
	n->isDir=isDir;
	n->content[0]='\0';
	n->parent=parent;
	n->firstChild=NULL;
	n->nextSibling=NULL;
	return n;
}

void freeTree(Node* n){
	Node* c=n->firstChild;
	while(c!=NULL){
		Node* next=c->nextSibling;
		freeTree(c);
		c=next;
	}
	delete n;
}

bool isStackEmpty(){
	return stackTop==0;
}

void pushFolder(Node* folder){
	if(stackTop<MAX_STACK){
		stackArr[stackTop++]=folder;
	}
}

Node* popFolder(){
	stackTop--;
	return stackArr[stackTop];
}

void clearStack(){
	stackTop=0;
}

bool isQueueEmpty(){
	return qCount==0;
}

void enqueueCmd(const char* cmd){
	if(qCount<MAX_QUEUE){
		strCopy(queueArr[qRear],cmd,LINE_LEN);
		qRear=(qRear+1)%MAX_QUEUE;
		qCount++;
	}
}

void dequeueCmd(char* out){
	strCopy(out,queueArr[qFront],LINE_LEN);
	qFront=(qFront+1)%MAX_QUEUE;
	qCount--;
}

void clearQueue(){
	qFront=0;
	qRear=0;
	qCount=0;
}

Node* findChild(const char* name){
	for(Node* c= cwd->firstChild;c!=0;c=c->nextSibling){
		if(strEq(c->name,name)) return c;
	}
	return NULL;
}

void addNode(const char* name, bool isDir){
	if(strLen(name)==0 || strLen(name) >=NAME_LEN){
		cout<<"Invalid name (max "<<NAME_LEN-1<<" characters)."<<endl;
		return;
	}
	if(findChild(name)!=NULL){
		cout<<"'"<<name<<"' already exists here."<<endl;
		return;
	}
	Node* n=newNode(name,isDir,cwd);
	if(cwd->firstChild==NULL){
		cwd->firstChild=n;
	} else{
		Node* last=cwd->firstChild;
		while(last->nextSibling!=NULL) last=last->nextSibling;
		last->nextSibling=n;
	}
	cout<<(isDir?"Folder":"File")<<" created: "<<name<<endl;
}

void printPath(Node* n){
	if(n->parent==0){
		cout<<"/";
		return;
	}
	if(n->parent->parent!=NULL) printPath(n->parent);
	cout<<"/"<<n->name;
}

void printTree(Node* folder, int depth){
	for(Node* c=folder->firstChild;c!=NULL;c=c->nextSibling){
		for(int i=0;i<depth;i++) cout<<"    ";
		cout<<"|-- "<<c->name;
		if(c->isDir) cout<<"/";
		cout<<endl;
		if(c->isDir) printTree(c,depth+1);
	}
}

void findRec(Node* folder, const char* pattern, int& found){
	for(Node* c=folder->firstChild;c!=NULL;c=c->nextSibling){
		if(contains(c->name,pattern)){
			cout<<"  ";
			printPath(c);
			cout<<endl;
			found++;
		}
		if(c->isDir) findRec(c,pattern,found);
	}
}

void showPrompt(){
	printPath(cwd);
	cout<<" $ ";
}

void pwdCmd(){
	printPath(cwd);
	cout<<endl;
}

void mkdirCmd(const char* name){
	addNode(name,true);
}

void touchCmd(const char* name){
	addNode(name,false);
}

void lsCmd(){
	if(cwd->firstChild==NULL){
		cout<<"(empty folder)"<<endl;
		return;
	}
	for(Node* c=cwd->firstChild;c!=NULL;c=c->nextSibling){
		if(c->isDir) cout<<"[DIR]  "<<c->name<<"/"<<endl;
		else cout<<"[FILE] "<<c->name<<endl;
	}
}

void cdCmd(const char* name){
	Node* target=NULL;
	if(strEq(name,"/")){
		target=root;
	} else if(strEq(name,"..")){
		target=(cwd->parent!=NULL)?cwd->parent:cwd;
	} else{
		Node* c=findChild(name);
		if(c==NULL){
			cout<<"No such folder: "<<name<<endl;
			return;
		}
		if(!c->isDir){
			cout<<name<<" is a file, not a folder."<<endl;
			return;
		}
		target=c;
	}
	if(target!=cwd){
		pushFolder(cwd);
		cwd=target;
	}
}

void backCmd(){
	if(isStackEmpty()){
		cout<<"No previous folder."<<endl;
		return;
	}
	cwd=popFolder();
	cout<<"Back to ";
	printPath(cwd);
	cout<<endl;
}

void writeCmd(const char* name, const char* text){
	Node* f=findChild(name);
	if(f==NULL){
		addNode(name,false);
		f=findChild(name);
		if(f==NULL) return;
	}
	if(f->isDir){
		cout<<name<<" is a folder."<<endl;
		return;
	}
	strCopy(f->content,text,LINE_LEN);
	cout<<"Saved "<<strLen(f->content)<<" characters."<<endl;
}

void catCmd(const char* name){
	Node* f=findChild(name);
	if(f==NULL){
		cout<<"No such file: "<<name<<endl;
		return;
	}
	if(f->isDir){
		cout<<name<<" is a folder."<<endl;
		return;
	}
	if(f->content[0]=='\0') cout<<"(empty file)"<<endl;
	else cout<<f->content<<endl;
}

void rmCmd(const char* name){
	Node* n=findChild(name);
	if(n==NULL){
		cout<<"No such file or folder: "<<name<<endl;
		return;
	}
	if(cwd->firstChild==n){
		cwd->firstChild=n->nextSibling;
	} else{
		Node* prev=cwd->firstChild;
		while(prev->nextSibling!=n) prev=prev->nextSibling;
		prev->nextSibling=n->nextSibling;
	}
	if(n->isDir) clearStack();
	freeTree(n);
	cout<<"Deleted "<<name<<endl;
}

void treeCmd(){
	cout<<"/"<<endl;
	printTree(root,0);
}

void findCmd(const char* pattern){
	int found=0;
	findRec(root,pattern,found);
	if(found==0) cout<<"  no matches."<<endl;
	else cout<<found<<" match(es)."<<endl;
}

void printHelp(){
	cout<<endl
		<<"Commands:"<<endl
		<<"  pwd              show current folder"<<endl
		<<"  ls               list current folder"<<endl
		<<"  cd <name>        enter a folder"<<endl
		<<"  back             go back to the previous folder (stack)"<<endl
		<<"  mkdir <name>     create a folder"<<endl
		<<"  touch <name>     create an empty file"<<endl
		<<"  write <f> <text> write text into a file"<<endl
		<<"  cat <file>       show file text"<<endl
		<<"  rm <name>        delete a file or folder"<<endl
		<<"  tree             show everything as a tree"<<endl
		<<"  find <text>      search all names "<<endl
		<<"  batch            queue commands; type 'run' to execute, 'cancel' to discard"<<endl
		<<"  demo             create a sample file system"<<endl
		<<"  help, exit"<<endl<<endl;
}

bool execute(const char* line, bool fromQueue);

void runQueue(){
	if(isQueueEmpty()){
		cout<<"Nothing queued."<<endl;
		return;
	}
	cout<<"Running "<<qCount<<" queued command(s) in order:"<<endl;
	char line[LINE_LEN];
	while(!isQueueEmpty()){
		dequeueCmd(line);
		cout<<"> "<<line<<endl;
		execute(line,true);
	}
}

bool execute(const char* line, bool fromQueue){
	char cmd[16], arg[LINE_LEN];
	const char* rest=readWord(line,cmd,16);
	rest=readWord(rest,arg,LINE_LEN);

	if(cmd[0]=='\0') return true;

	if(strEq(cmd,"exit")) return fromQueue;
	else if(strEq(cmd,"help")) printHelp();
	else if(strEq(cmd,"pwd")) pwdCmd();
	else if(strEq(cmd,"ls")) lsCmd();
	else if(strEq(cmd,"cd")){
		if(arg[0]!='\0') cdCmd(arg);
		else cdCmd("/");
	} else if(strEq(cmd,"back")) backCmd();
	else if(strEq(cmd,"mkdir")){
		if(arg[0]!='\0') mkdirCmd(arg);
		else cout<<"Usage: mkdir <name>"<<endl;
	} else if(strEq(cmd,"touch")){
		if(arg[0]!='\0') touchCmd(arg);
		else cout<<"Usage: touch <name>"<<endl;
	} else if(strEq(cmd,"write")){
		if(arg[0]!='\0' && rest[0]!='\0') writeCmd(arg,rest);
		else cout<<"Usage: write <file> <text>"<<endl;
	} else if(strEq(cmd,"cat")){
		if(arg[0]!='\0') catCmd(arg);
		else cout<<"Usage: cat <file>"<<endl;
	} else if(strEq(cmd,"rm")){
		if(arg[0]!='\0') rmCmd(arg);
		else cout<<"Usage: rm <name>"<<endl;
	} else if(strEq(cmd,"tree")) treeCmd();
	else if(strEq(cmd,"find")){
		if(arg[0]!='\0') findCmd(arg);
		else cout<<"Usage: find <text>"<<endl;
	} else if(strEq(cmd,"batch")){
		if(fromQueue){
			cout<<"Nested batch ignored."<<endl;
		} else{
			batchMode=true;
			cout<<"Batch mode: commands are queued, not run."<<endl<<"Type 'run' to execute them, or 'cancel' to discard."<<endl;
		}
	} else if(strEq(cmd,"demo")){
		clearQueue();
		enqueueCmd("mkdir docs");
		enqueueCmd("mkdir pics");
		enqueueCmd("cd docs");
		enqueueCmd("write notes.txt Remember to submit the DSA project");
		enqueueCmd("mkdir school");
		enqueueCmd("cd school");
		enqueueCmd("write dsa.txt Trees, stacks and queues");
		enqueueCmd("cd /");
		enqueueCmd("cd pics");
		enqueueCmd("touch photo.png");
		enqueueCmd("cd /");
		enqueueCmd("tree");
		runQueue();
		cout<<endl<<"Try: cd docs, ls, cat notes.txt, find txt, back, rm pics"<<endl;
	} else{
		cout<<"Unknown command '"<<cmd<<"'. Type 'help'."<<endl;
	}

	return true;
}

int main(){
	root=newNode("/",true,0);
	cwd=root;
	cout<<"==== Mini File System Simulator ===="<<endl<<"Type 'help' for commands or 'demo' for a sample."<<endl;
	char line[LINE_LEN];
	while(true){
		if(batchMode) cout<<"batch("<<qCount<<")> ";
		else showPrompt();

		cin.getline(line,LINE_LEN);
		if(!cin){
			if(cin.eof()) break;
			cin.clear();
			cin.ignore(10000,'\n');
			continue;
		}

		if(batchMode){
			if(strEq(line,"run")){
				batchMode=false;
				runQueue();
			} else if(strEq(line,"cancel")){
				batchMode=false;
				clearQueue();
				cout<<"Batch cancelled."<<endl;
			} else if(line[0]!='\0'){
				enqueueCmd(line);
				cout<<"  queued ("<<qCount<<")"<<endl;
			}
		} else{
			if(!execute(line,false)) break;
		}
	}

	freeTree(root);
	cout<<"Goodbye!"<<endl;
	return 0;
}