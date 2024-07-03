# ray-tracing

https://github.com/O-LEX/ray-tracing/assets/55784735/8480a5f3-6cf4-454b-b46e-61d30e34979d

Windowsを想定しています。
vcpkgをC:/dev/にインストールしてください。公式リポジトリに説明があります。
vcpkgを使ってglm,glfw3をインストールしてください。

externalフォルダを作りgladとtinygltfのjson.hpp,stb_image_write.h,stb_image.h,tiny_gltf.hを入れてください。

external/glad
external/tinygltf/json.hpp
external/tinygltf/stb_image_write.h
external/tinygltf/stb_image.h
external/tinygltf/tiny_gltf.h

のようになります。

macでもbrewを使ってglmとglfw3をインストールすることでmain1とmain2は動作させることができます。macのopenglのバージョンが古いために。main3,main4,main5はmacでは動作しません。

assetフォルダを作りそこにgltfファイルを入れ、mainの中で読み込むファイル名を指定してください。model()の中にパスを指定してください。読み込む際にテクスチャがあることを前提としているのでテクスチャがない場合は読み込めません。

main1 vertex shaderとfragment shaderを使って、gltfをそのまま描画するプログラムです。model.draw()を使って描画します。
main2 vertex shaderとfragment shaderを使って、gltfを描画するプログラムです。カラーは赤に設定しています。modelからトライアングルをすべて取ってきてvaoを作成し描画します。
main3 カメラからレイを飛ばして、オブジェクトと交差するかどうかを判定するプログラムです。bvhを使っています。カメラを動かせます。
main4 bvhを使わずに、すべてのトライアングルと交差判定を行うプログラムです。重いのでカメラは動かせません。
main5 レイとメッシュの交差点から光源方向にレイを飛ばします。遮るものがなければ明るさがでます。bvhを使っています。カメラを動かせます。本来であればマテリアルを設定して再帰的なサンプリングを行うべきでしょうが、やめました。
