window.onload = () => {
    getFirstItem()
    getSecondItem()  
}

getFirstItem = async() => {
    try{
        const response = await fetch("http://deelay.me/700/https://jsonplaceholder.typicode.com/todos/1")
        console.log(response.json())
        document.getElementById("item1").innerHTML = "<p>\
        A Universidade de São Paulo (USP) é uma das quatro universidades públicas mantidas pelo governo do estado brasileiro de São Paulo, junto com a Universidade Estadual de Campinas (Unicamp), Universidade Estadual Paulista (UNESP) e a Universidade Virtual do Estado de São Paulo (Univesp). \
        </p>"
    }catch(err){
        document.getElementById("item1").innerHTML = "<p>\
        Não foi possível recuperar o conteúdo\
        </p>"
    }
}

getSecondItem = async() => {
    try{
        const response = await fetch("http://deelay.me/1000/https://jsonplaceholder.typicode.com/photos")
        console.log(response.json())
        document.getElementById("item2").innerHTML = "<img src='https://upload.wikimedia.org/wikipedia/commons/thumb/2/2f/Webysther_20170627_-_Bras%C3%A3o_USP.svg/200px-Webysther_20170627_-_Bras%C3%A3o_USP.svg.png'/>"
    }catch(err){
        document.getElementById("item2").innerHTML = "<p>\
        Não foi possível recuperar o conteúdo\
        </p>"
    }
}
