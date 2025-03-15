# D23_NguyenHuuHoangAnh

## BÁO CÁO NGHIÊN CỨU LAB NGÀY 26/10/2024

### CÔNG VIỆC ĐÃ LÀM
#### 1. Nghiên cứu cách sử dụng GitHub
- [1]: http://example.com/ "Title"
[2]: http://example.org/ "Title"
- Tải SourceTree và clone repositor
##### 1.1 Một số cách trình bày cơ bản 
- In nghiêng
  -   *nội dung *  ---> *nội dung*
- In đậm
  - ** nội dung ** ---> **nội dung**
- chèn link
  - [mô tả link] ( link: http/vidu/com)
  ---> [mô tả link](vidu.com)
- Referenced Links
  - ```
      Some text with [a link][1] and another [link][2].
      [1]: http://example.com/ "Title"
      [2]: http://example.org/ "Title"
    ```
    Some text with [a link][1] and
another [link][2].

[1]: http://example.com/ "Title"
[2]: http://example.org/ "Title"
- chú thích
  - ```
      I am studying GitHub [^1]
      [^1]:A software is called Github
    ```
     ---> I am studying GitHub [^1]
      [^1]:A software is called Github
- 
  
##### 1.2 Cách chèn ảnh
- chèn ảnh
  - ```
    ảnh: ![mô tả ảnh ](ảnh.png)
    ```
  ---> ảnh: ![mô tả ảnh](ảnh.png)
- Referenced Images
  - ```
    	
    ảnh 1 : ![mô tả][1]
    [1]: path/to/image.png 
    ```
    ---> ảnh 1 : ![mô tả][1]

       [1]: robocon.png
- Linked Images
  - ```
      Linked image : [![mô tả link](image.png)](http://link.com)
    ```
    ---> Linked image : [![mô tả link](image.png)](http://link.com)
##### 1.3 Cách chèn code
- chèn code
  - ```
        ```cpp
            #define ledPin 4
            void setup(){
              pinMode(ledPin,OUTPUT); }
            void loop(){
              digitalWrite(ledPin,HIGH); delay(3000); digitalWrite(ledPin,LOW); delay(3000);
             }
        ```
    ```
    --->
    ```cpp
            #define ledPin 4
            void setup(){
              pinMode(ledPin,OUTPUT); }
            void loop(){
              digitalWrite(ledPin,HIGH); delay(3000); digitalWrite(ledPin,LOW); delay(3000);
             }
    ```
  - Chèn link code
    - ```
        [mô tả link code](đường dẫn đến file code: https://github.com/ptitopen/D23_NguyenHuuHoangAnh/blob/main.c)
      ```
      --->
      [mô tả link code](https://github.com/ptitopen-git/D23_NguyenHuuHoangAnh/blob/0406b2766e3a49dfd0a26c2ad30a4d2ed0d78cc6/Lab_Report_2024/Report%20of%2020241026/main.c)
##### 1.4 Cách tạo bảng
- ```
  |A | B|
  | -|----------|
  | 1| nội dung |
  | 2| nội dung |
  | 3| nội dung |
  ```
 ----> 
  |A | B|
  | -|----------|
  | 1| nội dung |
  | 2| nội dung |
  | 3| nội dung |

