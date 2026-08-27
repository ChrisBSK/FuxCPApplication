# Fuxophone

**Fuxophone** est un plug-in audio (Audio Unit) et une application Standalone pour macOS, développés avec le framework 
[JUCE](https://juce.com/), qui intègrent le solveur de contrepoint par contraintes **FuxCP** dans une interface graphique interactive.

Le plug-in permet de générer des contrepoints à partir d'un Cantus Firmus, en configurant les espèces, la tessiture 
(*pitch range*) de chaque voix, les fonctions de coûts, ainsi que la méthode de recherche et de minimisation du solveur.

> **Fuxophone est conçu exclusivement pour macOS.** Il n'est pas compatible avec Windows ou Linux. Le format Audio Unit 
> (AU) fonctionne uniquement dans les hôtes audio macOS compatibles AU — notamment **GarageBand**, mais aussi Logic Pro, 
> MainStage, etc.

---

## Compatibilité

| | |
|---|---|
| Système d'exploitation | macOS uniquement |
| Architectures supportées | Apple Silicon (ARM64) **et** Intel (x86_64) |
| Formats produits | Audio Unit (AU) + Application Standalone |
| Hôte AU testé | GarageBand |

Chaque architecture (Intel / Apple Silicon) doit être compilée séparément, avec un binaire Gecode qui lui est propre — 
voir la section [Installation](#installation) ci-dessous.

---

## Logiciels et versions utilisés

| Logiciel | Version | Rôle |
|---|---|---|
| macOS | 12.7.6 (testé) | Système cible |
| Xcode Command Line Tools | Apple Clang 14.0.0 (Xcode 14.x) | Compilateur C++ (clang), requis même sans l'IDE Xcode 
complet |
| CMake | ≥ 3.15 | Génération du projet de build |
| Git | — | Récupération du code source et des submodules |
| [JUCE](https://github.com/juce-framework/JUCE) | Inclus en submodule (`externals/JUCE`) | Framework audio / interface graphique |
| [FuxCP](https://github.com/dorian-genon/FuxCP) | Inclus en submodule (`externals/FuxCP`) | Solveur de contrepoint par contraintes |
| [Gecode](https://www.gecode.org/) | 6.2.0 | Solveur de programmation par contraintes utilisé par FuxCP |

>  Le développement de Fuxophone a été réalisé avec l'IDE [CLion](https://www.jetbrains.com/clion/) (2025.2.3), 
> mais cet IDE **n'est pas nécessaire** pour compiler le projet : les commandes ci-dessous n'utilisent que `cmake` en 
> ligne de commande, avec les Xcode Command Line Tools comme compilateur sous-jacent.

---

## Installation

### 1. Prérequis communs (Intel et Apple Silicon)

```bash
# Outils de compilation Apple
xcode-select --install

# CMake (via Homebrew)
brew install cmake
```

### 2. Récupérer le projet avec ses submodules

Le projet dépend de deux submodules Git (`JUCE` et `FuxCP`), il faut donc les récupérer explicitement :

```bash
git clone --recurse-submodules https://github.com/ChrisBSK/Fuxophone.git
cd Fuxophone
```

Si vous avez déjà cloné le dépôt sans l'option `--recurse-submodules` :

```bash
git submodule update --init --recursive
```

### 3. Installer Gecode

#### Sur Mac Apple Silicon (ARM64)

Rien à faire : un binaire Gecode 6.2.0 précompilé pour ARM64 est déjà fourni dans le dépôt (`externals/gecode-arm64/`), 
et le projet le détecte automatiquement.

#### Sur Mac Intel (x86_64)

Gecode doit être compilé et installé localement en tant que *framework* macOS :

```bash
# Téléchargez les sources de Gecode 6.2.0 depuis https://www.gecode.org/download.html
cd gecode-6.2.0
./configure --enable-framework --with-architectures=x86_64 --disable-examples --disable-qt
make
sudo make install
```

Cela installe `Gecode.framework` dans `/Library/Frameworks/`, où le projet ira le chercher automatiquement à la 
compilation.

### 4. Compiler le plug-in

Le projet fournit un `CMakePresets.json` avec un preset par architecture, qui fixe automatiquement 
`CMAKE_OSX_ARCHITECTURES` et isole chaque build dans son propre dossier.

#### Sur Mac Apple Silicon (ARM64)

```bash
cmake --preset arm64
cmake --build --preset arm64
```

#### Sur Mac Intel (x86_64)

```bash
cmake --preset x86_64
cmake --build --preset x86_64
```

À la fin de la compilation :
- le composant Audio Unit (`Fuxophone.component`) est automatiquement copié dans `~/Library/Audio/Plug-Ins/Components/`
- l'application Standalone est disponible dans `build-arm64/Fuxophone_artefacts/Standalone/Fuxophone.app` 
- (ou `build-x86_64/...` sur Intel)

### 5. Rafraîchir le cache Audio Unit de macOS

Après chaque nouvelle compilation, macOS doit être forcé à re-scanner les plug-ins Audio Unit installés :

```bash
killall -9 AudioComponentRegistrar
rm -rf ~/Library/Caches/AudioUnitCache
```

Vous pouvez ensuite vérifier que le plug-in est valide avec l'outil Apple `auval` :

```bash
auval -v aumu Fuxp Chba
```

---

## Utilisation dans GarageBand

1. Ouvrez GarageBand
2. Créez une piste **Instrument logiciel**
3. Dans l'éditeur d'instrument, choisissez **AU Instruments → ChrisBakashika → Fuxophone**
4. La fenêtre du plug-in Fuxophone s'ouvre : vous pouvez saisir un Cantus Firmus, configurer les voix et 
lancer la génération

## Utilisation en Standalone

Il n'est pas nécessaire de passer par un hôte audio : l'application Standalone se lance directement en ouvrant 
`Fuxophone.app` (voir chemin ci-dessus), avec un clavier virtuel MIDI intégré.

---

## Dépannage

**Le plug-in n'apparaît pas dans GarageBand après compilation**
Relancez la procédure de la section [5](#5-rafraîchir-le-cache-audio-unit-de-macos), puis redémarrez GarageBand.

**macOS refuse d'ouvrir le plug-in ("développeur non identifié" / Gatekeeper)**
Le plug-in est signé en local (signature *ad-hoc*), ce qui peut être bloqué par Gatekeeper sur d'autres 
machines que celle utilisée pour la compilation. Autorisez-le manuellement avec :

```bash
sudo spctl --add --label "Fuxophone" ~/Library/Audio/Plug-Ins/Components/Fuxophone.component
```

---

## Code Source

Le dossier `src/` contient le code source du plug-in. Les fichiers sont organisés en dossiers selon leur rôle dans 
l'architecture Contrôleur / Modèle / Vue / Service décrite dans le mémoire :

- **`src/plugin/`** : Point d'entrée du plug-in (`PluginProcessor`, `PluginEditor`), 
 qui fait le lien avec l'hôte audio (GarageBand) via l'API Audio Unit de JUCE et porte l'état persistant 
 de l'application entre les ouvertures/fermetures de la fenêtre.

- **`src/ui/`** : Tous les composants graphiques JUCE de l'interface — panneau principal, 
  en-tête de navigation, pages secondaires (Solver, About, Saved configurations), clavier virtuel, et 
  le panneau central de configuration des contrepoints avec ses sous-composants.

- **`src/controller/`** : Contient `AppController`, qui coordonne les échanges entre l'interface, 
  le modèle musical et le service de génération, sans jamais dépendre directement de FuxCP.

- **`src/model/`** : Classes représentant le problème musical manipulé par le solveur 
 (`CantusProblem`, `ConstraintSettings`), ainsi que les utilitaires de conversion entre notation MIDI et noms de notes.

- **`src/service/`** : Contient `GenerationService`, seule classe du plug-in directement dépendante de FuxCP : 
 elle construit le problème à partir du modèle, lance le solveur dans un thread séparé, et remonte le résultat à 
 l'interface.

- **`src/audio/`** : Synthèse sonore utilisée par le clavier virtuel intégré au plug-in (`SimpleSynth`).

---

## Crédits

FuxCP repose sur les travaux successifs de Thibaut Wafflard, Anton Lamotte, Luc Cleenewerk, Diego de Patoul, 
Tom Lai et Dorian Genon. L'interface Fuxophone (contrôleur, modèle, vue, service, plug-in) a été développée 
par **Chris Bakashika**.
