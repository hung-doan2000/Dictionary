/*
Bài có sự tham khảo tài liệu của k62 Việt Nhật
Tham khảo phần lập trình giao diện người dùng GUI
386715 từ
*/
////////////////
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gdk/gdkkeysyms.h>
#include "jrb.h"
#include "inc/btree.h"
#define WORD_MAX 250
#define MEAN_MAX 100000


void separate_mean (char* mean);
void initDict (char *filename);
void set_textView_text (char * text);
void Show_message (GtkWidget * parent , GtkMessageType type,  char * mms, char * content);
void close_window (GtkWidget *widget, gpointer window);

gboolean on_tab_key_press_event(GtkWidget *widget,GdkEventKey *pKey,gpointer user_data);//nhan tab
void jrb_to_list (JRB nextWordArray, int number); //tạo danh sách hiển thị bằng jrb
void suggest(char * word);  //gợi ý
gboolean search_suggest(GtkWidget * entry, GdkEvent * event, gpointer No_need);
int btfind (char * word);
void search (GtkWidget *w, gpointer data);
void cfedit(GtkWidget *w, gpointer data);
void edit( GtkWidget *w, gpointer data); //dùng cho phần cập nhập chỗ tra từ
void tra_tu (GtkWidget widget, gpointer window);

void add(GtkWidget *w, gpointer data);
void them_tu (GtkWidget widget, gpointer window);


void del (GtkWidget *w, gpointer data);
void confirmdel(GtkWidget *w, gpointer data);
void xoa_tu (GtkWidget widget, gpointer window);


void about (GtkWidget widget, gpointer window);


BTA *dictionary = NULL;

const gchar *a, *b; //kieu char giao dien nguoi dung
GtkWidget *textView, *view1;
GtkWidget *about_dialog, *entry_search, *entry; 
GtkListStore *list; //tạo ra list lưu trữ để hiển thị
GtkWidget *window2;

////////////SOURE CODE
///

void destroy_something(GtkWidget * widget, gpointer gp) {
	gtk_widget_destroy(gp);
}

void initDict (char *filename){
    FILE *datafile;
    dictionary = btcrt("evdic.dat", 0, 0);
	datafile = fopen(filename, "r");  //mở file
	if (datafile == NULL){
		printf("Cannot open file\n");
		exit(1);
	}
    	char word[WORD_MAX], mean[MEAN_MAX];

    while (fscanf(datafile, "%[^@]", word) == 1){
        fgets(mean, MEAN_MAX, datafile);
        separate_mean(mean);
        btins(dictionary, word, mean, strlen(mean) + 1);
    }
    fclose(datafile);
}

void separate_mean (char* mean){	
    int i = 0, j = 1;
    while (mean[j] != '\0'){
        if (mean[j] == '	'){
            mean[i++] = '\n';
            j += 2;
        }
        else{
            if(i != j)
                mean[i++] = mean[j++];
            else{
                i++; j++;
            }
	    }
	}
	mean[i] = '\0';
}



void set_textView_text (char * text){ //dùng hiển thị nghĩa
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView)); //dua tu vao bo nho dem
	if (buffer == NULL)
		buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, text, -1);  //tạo ra đoạn text buffer
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(textView), buffer); // hiển thị đoạn text được lưu ở vùng nhớ đệm
}

void Show_message (GtkWidget *parent , GtkMessageType type,  char * mms, char * content){ //dòng 1 là mms, dòng 2 là content
	GtkWidget *mdialog;
	mdialog = gtk_message_dialog_new(GTK_WINDOW(parent), GTK_DIALOG_DESTROY_WITH_PARENT, type, GTK_BUTTONS_OK, "%s", mms);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(mdialog), "%s",  content);
	gtk_dialog_run(GTK_DIALOG(mdialog));
	gtk_widget_destroy(mdialog);
}

void close_window (GtkWidget *widget, gpointer window){ // đóng cửa sổ
	gtk_widget_destroy(GTK_WIDGET(window));
}

gboolean on_tab_key_press_event(GtkWidget *widget,GdkEventKey *pKey,gpointer user_data){
	const gchar *word;
  	int d,size;
  	char s1[1000],s2[10000];
    if (pKey) {
    	if(pKey->keyval == GDK_KEY_Tab){  //nếu nhấn nút tab
    		word = gtk_entry_get_text(GTK_ENTRY(entry_search));
      	 	btpos(dictionary,1); 
      		while((d = btseln(dictionary,s1,s2,10000,&size))==0)
        	if(strncmp((char*)word,s1,strlen((char*)word))==0){//so sanh strlen((char*)word) ki tu dau cua word voi s1
          		gtk_entry_set_text(GTK_ENTRY(entry_search),s1); //nhập s1 vào entry search
          		break;
          	} 
        }
  	return FALSE;
  	}
  	return TRUE;
}

void jrb_to_list (JRB nextWordArray, int number){ //dùng jrb để hiển thị gợi ý chữ tiếp theo
    GtkTreeIter Iter;  //Iter viết tắt của Iterator là vòng lặp ??
    JRB tmp;
    jrb_traverse(tmp, nextWordArray) { //duyệt cây jrb chữ tiếp theo lưu vào cây jrb tmp
     	gtk_list_store_append(GTK_LIST_STORE(list), &Iter);  //nối thêm từ vào liststore
     	gtk_list_store_set(GTK_LIST_STORE(list), &Iter, 0, jval_s(tmp->key), -1 ); // thêm key vào jrb
    }
}


void suggest(char * word){   //JRB de hien danh sach
	char nextword[100], prevword[100];  //kí tự trước và sau
	int i; 
	int max = 10;
	GtkTreeIter Iter; //iterator vòng lặp
	JRB tmp, nextWordArray = make_jrb();  //tạo cây jrb chuỗi từ tiếp theo
	BTint value, found = 0;  //kiểu int của BTint
	strcpy(nextword, word);  //copy word vào nextwword
	int wordlen = strlen(word);  //độ dài của từ đang nhập
	gtk_list_store_clear(GTK_LIST_STORE(list));  //clear list
	if (bfndky(dictionary, word, &value) ==  0) {  // tim word trong dictionary, value la gia tri cua 'word' tim duoc
		found = 1;     //Kiểu BTint nếu tìm được word trong dictionary thì bằng 1
		gtk_list_store_append(GTK_LIST_STORE(list), &Iter); //thêm vào list
		gtk_list_store_set(GTK_LIST_STORE(list), &Iter, 0, nextword, -1 ); // neu dung thi ok
	}
	else     // Nếu k tồn tại thì thêm nextwword vào Btree
		btins(dictionary, nextword, "", 1); // chen key va data vao B-tree, o lay chen blank vao !

	for (i = 0; i < max; i++) {      
		bnxtky(dictionary, nextword, &value);  // tim 'key' tiep theo, trả về giá trị tiếp theo của nextword tìm được trong 
			jrb_insert_str(nextWordArray, strdup(nextword), JNULL);  //chèn từ gần giống vào NextWord
	}

	jrb_to_list(nextWordArray, i);  // chèn  nextwordArray vào list
	if (!found)     //nếu k tồn tại found thì xoá từ word trong dictionary
		btdel(dictionary, word);
	jrb_free_tree(nextWordArray);  //gỉai phóng chuỗi từ tiếp theo
}

gboolean search_suggest(GtkWidget *entry, GdkEvent * event, gpointer No_need){ //gdkEvent nhận ra phím mình bấm là gì ví dụ enter dể tìm
	GdkEventKey *keyEvent = (GdkEventKey *)event;  //kiẻu dữ liệu event luôn có thể truy cập k cần quan tâm đó là kiểu gì
	char word[50];
	int len;
	strcpy(word, gtk_entry_get_text(GTK_ENTRY(entry_search)));  //bấm enter sau khi đã nhập và copy từ nhập vào word
	if (keyEvent->keyval != GDK_KEY_BackSpace) { //nếu khác xoá
		len = strlen(word);
		word[len] = keyEvent->keyval;  
		word[len + 1] = '\0';
	}
	else {
		len = strlen(word); 
		word[len - 1] = '\0';
	}
	suggest(word);
	return FALSE;
}

int btfind (char * word){ //hàm tìm từ trong cây btree
	char mean[100000];
	int size;
	if (btsel(dictionary,word,mean,100000,&size)==0){
		set_textView_text(mean);
	    g_print("%s",mean); //in mean tư buffer
		return 1;
	}
    else {
    	return 0;
    }
}

void search(GtkWidget *w, gpointer data){
	GtkWidget *entry1= ((GtkWidget**)data)[0];   //ô nhập
	GtkWidget *window1=((GtkWidget**)data)[1];    //cửa sổ
	char word[50];

	strcpy(word,gtk_entry_get_text(GTK_ENTRY(entry_search)));   //copy a vào word
	if (word[0] == '\0')  //nếu word đầu là kí tự kết thúc câu nghĩa là chưa nhập gì cả
		Show_message(window1, GTK_MESSAGE_WARNING, "Warning!", "Enter word to search!");
	else {
		int result = btfind(word);   //tìm kiếm từ word
		if (result==0) //btfind trả về 0, không tìm được kí tự   
			Show_message(window1,GTK_MESSAGE_ERROR, "Error!","Cannot found in dictionary!");
	}
	return;
}

void edit( GtkWidget *w, gpointer data){  // sửa từ
	GtkWidget *entry1= ((GtkWidget**)data)[0];   // ô nhập
	GtkWidget *window1=((GtkWidget**)data)[1];  //cửa sổ
	GtkWidget *edit_view=((GtkWidget**)data)[2];  //ô hiện lên sửa

	BTint x; //kiểu int của gtk được giới hạn bằng G_MININT và G_MAXINT

	if (gtk_entry_get_text(GTK_ENTRY(entry_search))[0] == 0 || bfndky(dictionary, (char*)gtk_entry_get_text(GTK_ENTRY(entry_search)), &x) != 0){  //nếu k có giá trị nhập vào hoặc khi tìm khoá trong entry1 dictionary trả về biến khac 0 nghĩ là việc tìm thất bại
		Show_message(window1, GTK_MESSAGE_INFO, "Tutorial!", "Need search before editing!"); // show_message o tren
		return;
	}

	char word[50], mean[100000];
	strcpy(word,gtk_entry_get_text(GTK_ENTRY(entry_search)));     //copy a vào word

	GtkTextBuffer *buffer2;    //bộ đệm văn bản
	GtkTextIter start, end, iter;

	buffer2 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GTK_TEXT_VIEW(textView)));  //đưa từ ở ô tìm kiếm vào bufer
	gtk_text_buffer_get_iter_at_offset(buffer2, &iter, 0); //

	gtk_text_buffer_insert(buffer2, &iter, "", -1);
	gtk_text_buffer_get_bounds (buffer2, &start, &end);
	//nhập nghĩa vàp b

	strcpy(mean,gtk_text_buffer_get_text (buffer2, &start, &end, FALSE));    //copy b vào nghĩa

	if (word[0] == '\0' || mean[0] == '\0') //nếu word rỗng và mean rỗng
		Show_message(window1, GTK_MESSAGE_WARNING, "Warning!", "No part is left blank!");
	else if (bfndky(dictionary, word, &x ) != 0)
		Show_message(window1, GTK_MESSAGE_ERROR, "Error!", "Word not found!");
	else
	{
		if( btupd(dictionary, word, mean, strlen(mean) + 1)==1) //cập nhập thất bại
			Show_message(window1,GTK_MESSAGE_ERROR, "Error!","Cannot update!");
		else
			Show_message(window1,GTK_MESSAGE_INFO, "Success!","Updated!");
	}
	gtk_window_close(GTK_WINDOW(window2)); //thoat cua so thong bao
}

void confirmedit(GtkWidget *w, gpointer data){   //xác nhận có sửa k 
	GtkWidget *fixed;
	GtkWidget *button1, *button2, *label;
	
	window2 = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
	gtk_window_set_title(GTK_WINDOW(window2), "");
	gtk_window_set_default_size(GTK_WINDOW(window2), 200, 100);
	gtk_window_set_position(GTK_WINDOW(window2), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(window2), FALSE);

	fixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window2), fixed);

	label = gtk_label_new("Are you sure ?");
	gtk_fixed_put(GTK_FIXED(fixed), label, 50, 20);

	button1 = gtk_button_new_with_label("Yes");
	gtk_fixed_put(GTK_FIXED(fixed), button1, 30, 50);
	gtk_widget_set_size_request(button1, 60, 30);

	button2 = gtk_button_new_with_label("No");
	gtk_fixed_put(GTK_FIXED(fixed), button2, 110, 50);
	gtk_widget_set_size_request(button2, 60, 30);

	g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(edit), data);
	g_signal_connect(G_OBJECT(button2), "clicked", G_CALLBACK(close_window), window2);

	gtk_widget_show_all(window2);
}


void add(GtkWidget *w, gpointer data){ 
	GtkWidget *entry1 = ((GtkWidget**)data)[0];
	GtkWidget *window1 = ((GtkWidget**)data)[1];
	GtkWidget *view1 = ((GtkWidget**)data)[2];

	GtkTextBuffer *buffer1;
	GtkTextIter start, end, iter;

	BTint x;
	char mean[10000], word[50];


	strcpy(word,gtk_entry_get_text(GTK_ENTRY(entry1)));

	buffer1 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GTK_TEXT_VIEW(view1))); 
	gtk_text_buffer_get_iter_at_offset(buffer1, &iter, 0);

	gtk_text_buffer_insert(buffer1, &iter, "", -1);
	gtk_text_buffer_get_bounds (buffer1, &start, &end);
	b = gtk_text_buffer_get_text (buffer1, &start, &end, FALSE);

	strcpy(mean,b);

	if (word[0] == '\0' || mean[0] == '\0')
		Show_message(window1, GTK_MESSAGE_WARNING, "Warning!", "No part is left blank!");
	else if (bfndky(dictionary, word, &x ) == 0)
		Show_message(window1, GTK_MESSAGE_ERROR, "Error!", "Word already in dictionary!");
	else
	{
		if(btins(dictionary,word, mean,10000))
			Show_message(window1,GTK_MESSAGE_ERROR, "Error!","Cannot add word!");
		else
			Show_message(window1,GTK_MESSAGE_INFO, "Success!","Done!");
	}

	return;
}


void them_tu (GtkWidget widget, gpointer window){
	GtkWidget *fixed, *button2;
	GtkWidget *button1,*window1,*label1,*entry1,*label2,*entry2;

	window1 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window1), "Add word");
	gtk_window_set_default_size(GTK_WINDOW(window1), 750, 450);
	gtk_window_set_position(GTK_WINDOW(window1), GTK_WIN_POS_CENTER);

	fixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window1), fixed);

	label1 = gtk_label_new("Word:");
	gtk_fixed_put(GTK_FIXED(fixed), label1, 30, 30);
	
	entry = gtk_entry_new();
	gtk_widget_set_size_request(entry, 300, 30);
	gtk_fixed_put(GTK_FIXED(fixed), entry, 100, 25);
	gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
	gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter word here");

	label2 = gtk_label_new("Meaning:");
	gtk_fixed_put(GTK_FIXED(fixed), label2, 30, 200);

	GtkWidget *scrolling = gtk_scrolled_window_new(NULL, NULL);
	gtk_fixed_put(GTK_FIXED(fixed), scrolling, 100, 100);
	gtk_widget_set_size_request(scrolling, 300, 300);

	view1 = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(scrolling), view1);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view1), GTK_WRAP_WORD);

	button1 = gtk_button_new_with_label("Add");
	gtk_fixed_put(GTK_FIXED(fixed), button1, 450, 15);
	gtk_widget_set_size_request(button1, 90, 30);

	button2 = gtk_button_new_with_label("Back");
	gtk_fixed_put(GTK_FIXED(fixed), button2, 560, 15);
	gtk_widget_set_size_request(button2, 90, 30);

	GtkWidget *data[3];
	data[0] = entry;
	data[1] = window1;
	data[2] = view1;

	g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(add), data);
	g_signal_connect(G_OBJECT(button2), "clicked", G_CALLBACK(close_window), window1);
	g_signal_connect(G_OBJECT(window1), "destroy", G_CALLBACK(destroy_something), window1);

	gtk_widget_show_all(window1);
	gtk_main();
}

void del (GtkWidget *w, gpointer data){
	GtkWidget *entry1= ((GtkWidget**)data)[0];   //tạo widget entry1
	GtkWidget *window1=((GtkWidget**)data)[1];   //widget window1

	char mean[10000], word[50];
	BTint x;
	int size;

	strcpy(word,gtk_entry_get_text(GTK_ENTRY(entry_search)));

	if (word[0] == '\0')
		Show_message(window1, GTK_MESSAGE_WARNING, "Warning!", "No part is left blank!");
	else if (bfndky(dictionary, word, &x ) != 0)
		Show_message(window1, GTK_MESSAGE_ERROR, "Error!", "Word not found!");
	else
		if(btsel(dictionary,word,mean,100000, &size) == 0) {
			btdel(dictionary,word);
			Show_message(window1,GTK_MESSAGE_INFO, "Success!","Done!");
		}
		else
			Show_message(window1,GTK_MESSAGE_ERROR, "Error!","Can not delete!");
    gtk_window_close(GTK_WINDOW(window2));
}

void confirmdel(GtkWidget *w, gpointer data){
	GtkWidget *fixed;
	GtkWidget *button1, *button2, *label;
	
	window2 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window2), "");
	gtk_window_set_default_size(GTK_WINDOW(window2), 200, 100);
	gtk_window_set_position(GTK_WINDOW(window2), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(window2), FALSE);

	fixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window2), fixed);

	label = gtk_label_new("Are you sure?");
	gtk_fixed_put(GTK_FIXED(fixed), label, 60, 20);

	button1 = gtk_button_new_with_label("Yes");
	gtk_fixed_put(GTK_FIXED(fixed), button1, 30, 50);
	gtk_widget_set_size_request(button1, 60, 30);

	button2 = gtk_button_new_with_label("No");
	gtk_fixed_put(GTK_FIXED(fixed), button2, 110, 50);
	gtk_widget_set_size_request(button2, 60, 30);

	g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(del), data);
	g_signal_connect(G_OBJECT(button2), "clicked", G_CALLBACK(close_window), window2);

	gtk_widget_show_all(window2);
}

void xoa_tu (GtkWidget widget, gpointer window){
	GtkWidget *fixed;
	GtkWidget *button1,*window1,*label,*button2;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Delete word");
	gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	fixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window), fixed);

	label = gtk_label_new("Input:");
	gtk_fixed_put(GTK_FIXED(fixed), label, 30, 30);

	entry = gtk_entry_new();
	gtk_fixed_put(GTK_FIXED(fixed), entry, 100, 25);
	gtk_widget_set_size_request(entry, 200, 30);
	gtk_entry_set_max_length(GTK_ENTRY(entry),50);
	gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter word here");

	GtkEntryCompletion *comple = gtk_entry_completion_new(); 
	gtk_entry_completion_set_text_column(comple, 0);
	list = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_entry_completion_set_model(comple, GTK_TREE_MODEL(list));
	gtk_entry_set_completion(GTK_ENTRY(entry), comple);

	button1 = gtk_button_new_with_label("Delete");
	gtk_fixed_put(GTK_FIXED(fixed), button1, 350, 15);
	gtk_widget_set_size_request(button1, 80, 30);

	GtkWidget *data[2];
	data[0]= entry;
	data[1]= window;

	g_signal_connect(entry, "key-press-event", G_CALLBACK(search_suggest), NULL);
	g_signal_connect(G_OBJECT(entry), "key-press-event", G_CALLBACK(on_tab_key_press_event), NULL);
	g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(confirmdel), data);
	g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(confirmdel), data);

	button2 = gtk_button_new_with_label("Back");
	gtk_fixed_put(GTK_FIXED(fixed), button2, 450, 15);
	gtk_widget_set_size_request(button2, 80, 30);

	g_signal_connect(G_OBJECT(button2), "clicked", G_CALLBACK(destroy_something), window);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_something), window);
	
	gtk_widget_show_all(window);
	gtk_main();
	return;
}

void about (GtkWidget widget, gpointer window){
	GtkWidget *fixed;
	about_dialog = gtk_about_dialog_new();

	fixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER((GtkWidget*)about_dialog), fixed);
	gtk_window_set_position(GTK_WINDOW(about_dialog), GTK_WIN_POS_CENTER);

	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), "Từ điển Anh Việt");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog), "Lê Minh An \n Nguyễn Cảnh Nam \n Đoàn Xuân Hưng\n Lê Thế Tài\n"); 
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog), " Việt Nhật 1 - K63");
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(about_dialog), NULL);
	gtk_dialog_run(GTK_DIALOG(about_dialog));
	gtk_widget_destroy(about_dialog);
}


int main(int argc, char *argv[]){
	btinit();
	initDict("AnhViet.txt");

	 //GTK+
    GtkWidget *window;
	GtkWidget *fixed, *image;
	GtkWidget *button1,*button2,*button3,*button4,*button5,*button6;
	GtkWidget *label,*label1,*label2;
	gtk_init(&argc, &argv);

	GtkWidget *data[3];
	data[0]= entry_search;
	data[1]= window;
	data[2]= textView;

	//tao cua so
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
	gtk_window_set_title(GTK_WINDOW(window), "English-Vietnamese Dictionary");
	gtk_window_set_default_size(GTK_WINDOW(window), 750, 480);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    //tao nen
	fixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window), fixed);

//	image = gtk_image_new_from_file("background.jpg");
//	gtk_container_add(GTK_CONTAINER(fixed), image);


	label = gtk_label_new("Input:");  //tao 1 o nhap trong window1
	gtk_fixed_put(GTK_FIXED(fixed), label, 30, 30);

	entry_search = gtk_entry_new();  //GtkWidget cục bộ được khai báo ở trên
	gtk_widget_set_size_request (entry_search, 300, 30); //ô nhập chiều rộng 300 cao 30
	gtk_fixed_put(GTK_FIXED(fixed), entry_search, 100, 25);
	gtk_entry_set_max_length (GTK_ENTRY(entry_search),100); //độ dài từ nhập 100
	gtk_entry_set_placeholder_text(GTK_ENTRY(entry_search), "Enter word here");
	
	//danh sach cac goi y khi tim kiem
	GtkEntryCompletion *comple = gtk_entry_completion_new();
	gtk_entry_completion_set_text_column(comple, 0);
	list = gtk_list_store_new(1, G_TYPE_STRING);  //1-số trường hay số cột trong bảng, kiểu dữ liệu của trường là String
	gtk_entry_completion_set_model(comple, GTK_TREE_MODEL(list));
	gtk_entry_set_completion(GTK_ENTRY(entry_search), comple);

	g_signal_connect(G_OBJECT(entry_search), "key-press-event", G_CALLBACK(search_suggest), NULL);  //goi tim kiem suggest
	g_signal_connect(G_OBJECT(entry_search), "key-press-event", G_CALLBACK(on_tab_key_press_event), NULL); //bam tab hoan thanh tu
	g_signal_connect(G_OBJECT(entry_search), "activate", G_CALLBACK(search), data);     //goi search

	button1 = gtk_button_new_with_label("Search");
	gtk_fixed_put(GTK_FIXED(fixed), button1, 450, 20);
	gtk_widget_set_size_request(button1, 180, 30);
	g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(search), data);   //data hàm trỏ vào dữ liệu

	label2 = gtk_label_new("Meaning:");
	gtk_fixed_put(GTK_FIXED(fixed), label2, 30, 200);
	
	GtkWidget *scrolling = gtk_scrolled_window_new(NULL, NULL);
	gtk_fixed_put(GTK_FIXED(fixed), scrolling, 100, 80);
	gtk_widget_set_size_request(scrolling, 300, 420);
		
	textView = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(scrolling), textView);

	button2 = gtk_button_new_with_label("Update meaning");
	gtk_fixed_put(GTK_FIXED(fixed), button2, 450, 70);
	gtk_widget_set_size_request(button2, 180, 30);
	g_signal_connect(G_OBJECT(button2), "clicked", G_CALLBACK(confirmedit), data);  //hàm sửa - edit sửa nghĩa của từ

	button3 = gtk_button_new_with_label("Add word");
	gtk_fixed_put(GTK_FIXED(fixed), button3, 450, 120);
	gtk_widget_set_size_request(button3, 180, 30);
	gtk_widget_set_tooltip_text(button3, "Add a word");
	g_signal_connect(G_OBJECT(button3), "clicked", G_CALLBACK(them_tu), NULL);  //goi ham them tu

	button4 = gtk_button_new_with_label("Delete word");
	gtk_fixed_put(GTK_FIXED(fixed), button4, 450, 170);
	gtk_widget_set_size_request(button4, 180, 30);
	gtk_widget_set_tooltip_text(button4, "Delete a word");
	g_signal_connect(G_OBJECT(button4), "clicked", G_CALLBACK(xoa_tu), NULL);  //goi ham xoa tu

	button5 = gtk_button_new_with_label("Our team");
	gtk_fixed_put(GTK_FIXED(fixed), button5, 450, 220); // đánh toạ độ cho nút
	gtk_widget_set_size_request(button5, 180, 30);
	g_signal_connect(G_OBJECT(button5), "clicked", G_CALLBACK(about), NULL); //thong tin

	button6 = gtk_button_new_with_label("Exit");
	gtk_fixed_put(GTK_FIXED(fixed), button6, 450, 270);
	gtk_widget_set_size_request(button6, 180, 30);
	gtk_widget_set_tooltip_text(button6, "Exit");
	g_signal_connect(G_OBJECT(button6), "clicked", G_CALLBACK(gtk_main_quit),NULL); //gọi hàm thoát chương trình

	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_show_all(window);
	gtk_main();
	btcls(dictionary);

	return 0;
}


