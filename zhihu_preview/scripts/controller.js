angular.module('zhihu', [])
	.controller('zhihuPreviewCtrl', ['$scope', '$http',
		function($scope, $http) {
			$http.get('invite_panel.json').success(function(data) {
				$scope.comment=data.comment
				$scope.invited = data.invited
				$scope.recommended = data.recommended
			})
			
			$scope.persons=[]

			$scope.$watch(
				function($scope) {
					$scope.count = 0
					

					for (i = 0; i < $scope.invited.length; i++) {
						if ($scope.invited[i].toggle == true) {
							$scope.count++;
							$scope.persons.push($scope.invited[i])
						}else{
							$scope.persons.splice(i)
						}
					}

					for (i = 0; i < $scope.recommended.length; i++) {
						if ($scope.recommended[i].toggle == true) {
							$scope.count++;
							$scope.persons.push($scope.recommended[i])
						}
					}
				}
			);

		}
	]);