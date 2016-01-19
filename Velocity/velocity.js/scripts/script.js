(function ($) {
    // $('#div1').velocity({
    // 	width :'300px'
    // },{
    // 	duration:1000,
    // 	complete:function(){
    // 		$('#div2').velocity({
    // 			width :'300px',
    // 			height :'300px'
    // 		},{
    // 			duration:1000
    // 		});
    // 	}
    // });

    // var seq=[
    // {
    // 	elements: $('#div1'),
    // 	properties:{width :'300px'},
    // 	options:{duration:1000}
    // },{
    // 	elements: $('#div2'),
    // 	properties:{width :'300px'},
    // 	options:{duration:1000}
    // },{
    // 	elements: $('#div3'),
    // 	properties:{width :'300px'},
    // 	options:{duration:1000}
    // }];

    // $.Velocity.RunSequence(seq);

    $('#div1').on('mouseover', function () {
        $(this).velocity('callout.shake');
    });

    $.Velocity.RegisterEffect('lixin.pulse', {
        defaultDuration: 300,
        calls: [
            [{scaleX: 1.1}, 0.5],
            [{scaleX: 1.0}, 0.5]
        ]
    });

    $('#div2').on('mouseover', function () {
        $(this).velocity('lixin.pulse');
    });

})(jQuery);